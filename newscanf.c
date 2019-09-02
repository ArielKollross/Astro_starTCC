/* Rotina para a leitura de dados e rotinas derivadas */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PI        3.1415926535897932384626433832795029 /* o valor de pi */
#define PI2       2*PI       /* O dobro de \pi                           */

FILE *fi;

int newscanf(FILE *arq, char *fmt, ...) 
/* Routina de leitura de dados em formato flexivel admitindo a existência de
 * comentários inseridos que não devem ser lidos. Seu uso é análogo ao da
 * rotina scanf tradicional (man 3 scanf), exceto que caracteres que não sejam
 * especificações de formato no string serão ignorados. Foi acrescido o
 * formato %S que permite ler um string, tal qual %s, mas com os eventuais
 * comentários antes do inicio do string. Os caracteres de comentário são os
 * usuais em diversas linguagens, a saber: # % ! ; (Agradecimentos ao Carlos
 * de Carvalho por te-la me passado). */
{
  va_list    ap;         /* ponteiro para os argumentos */
  int        lidos = 0;
  int        S     = 0;
  int        ret;
  void      *valor;
  char      *p;
  char       save;
  char      *locfmt;
  char      *svloc;
  const char conversoes[] = "diuoxcSspfeg%[";
  
  /* Deve-se copiar pois `fmt` é read-only :-( */
  if ((svloc = locfmt = (char*) malloc(strlen(fmt) + 1)) == NULL) 
  {
    fprintf(stderr, "\n(newscanf) memoria exaurida \n");
    return lidos;
  }
  strcpy(locfmt, fmt);
  
  va_start(ap, fmt); /* 'ap' aponta pra primeira variável a ser lida */
  
  for ( ; *locfmt; locfmt++) 
  {
    if (*locfmt != '%') /* Ignora o que não é especificação de formato */
      continue;
    
    p = strpbrk(locfmt + 1, conversoes);
    if (!p || *p == '%') 
    {
      fprintf(stderr, "\n(newscanf) Formato inválido \n");
      goto fim;
    }
    
    if ( *p == '[' && !(p = strchr(p, ']')) ) 
    {
      fprintf(stderr, "\n(newscanf) Formato inválido \n");
      goto fim;
    }
    
    save = p[1];
    p[1] = '\0';
    
    switch (*p) 
    {
      case 'S':
	valor  = va_arg(ap, char *);
	*p = 's'; S = 1;
	break;
      case 'c': 
      case 's': 
      case ']':
	valor  = va_arg(ap, char *);
	break;
      case 'd': 
      case 'i': 
      case 'o': 
      case 'u': 
      case 'x':
	switch (p[-1]) 
	{
	  case 'h':
	    valor = va_arg(ap, short *);
	    break;
	  case 'l':
	    valor = va_arg(ap, long *);
	    break;
	  default:
	    valor = va_arg(ap, int *);
	}
	break;
      case 'p':
	valor = va_arg(ap, void **);
	break;
      case 'e': 
      case 'f': 
      case 'g':
	switch (p[-1]) 
	{
	  case 'l':
	    valor = va_arg(ap, double *);
	    break;
	  case 'L':
	    valor = va_arg(ap, long double *);
	    break;
	  default:
	    valor = va_arg(ap, float *);
	}
	if (!valor) 
	{
	  fprintf(stderr,"\n(newscanf) Erro endereço variável a ser lida\n");
	  goto fim;
	}
    }
    if (S) 
    {
      /* lê um string com %s mas remove comentários antes */
      for (;;) 
      {
	/* remove whitespace */
	while ((ret = getc(arq)) != EOF && isspace(ret))
	  ;
	if (ret == EOF) 
	{
	  if (!feof(arq))
	    fprintf(stderr, "\n(newscanf) erro de leitura \n");
	  goto fim;
	}
	/* se for comentário joga fora o resto da linha */
	if (ret == '#' || ret == ';' || ret == '!') 
	{
	  while ((ret = getc(arq)) != EOF && ret != '\n')
	    ;
	  if (ret != '\n') 
	  {
	    /* não achou o fim da linha */
	    if (!feof(arq))
	      fprintf(stderr, "\n(newscanf) erro de leitura \n");
	    goto fim;
	  }
	} 
	else
	  break;
      }
      /* lê o string */
      ungetc(ret, arq);
      if ((ret = fscanf(arq, locfmt, valor)) < 1) 
      {
	fprintf(stderr, "\n(newscanf) não foi possível ler um string \n");
	goto fim; /* se não conseguiu ler com %s, então tem galho */
      }
      S = 0;
    } 
    else 
    {
      while ((ret = fscanf(arq, locfmt, valor)) < 1) 
      {
	if (ret == EOF)
	  goto fim;
	
	/* se não for início de comentário é um erro */
	if ((ret = getc(arq)) < 0) 
	{
	  fprintf(stderr, "\n(newscanf) erro de leitura no arquivo \n");
	  goto fim;
	}
	if (ret != '#' && ret != '%' && ret != ';' && ret != '!') 
	{
	  fprintf(stderr, "\n(newscanf) dados incompatíveis com formato \n");
	  goto fim;
	}
	
	/* comentário: lê o resto da linha */
	while ((ret = getc(arq)) != EOF && ret != '\n')
	  ;
	if (ret != '\n') 
	{
	  /* não achou o fim da linha */
	  if (!feof(arq))
	    fprintf(stderr, "\n(newscanf) erro de leitura \n");
	  goto fim;
	}
      }
    }
    lidos++;
    p[1] = save;
    locfmt = p;
  }
 fim:
  free(svloc);
  va_end(ap);
  return lidos;
}

double newscanf_le_numero(int verbose, char *string) 
{
  double value;
  if ( verbose == 1 ) printf("%-40.40s = ",string);
  newscanf(fi,"%lf",&value);
  if ( verbose == 1 ) printf("%+-18.11g ",value);
  return value;
}

double newscanf_le_unidade(int verbose, char u[], int opcao) 
{
  if ( opcao == 1 ) 
  {
    newscanf(fi,"%S",u);
    if ( verbose == 1 ) if ( strncmp(".",u,strlen(u))!=0 ) printf("%-4.4s ",u);
  }
  
  /* secção de conversão da unidade para fator numérico de conversão ao SI */
  /* o adimensional */
  if ( strncmp("."   ,u,strlen(u))==0 ) return 1.0     ;
  /* unidades de comprimento */
  if ( strncmp("km"  ,u,strlen(u))==0 ) return 1.0e+03;
  if ( strncmp("m"   ,u,strlen(u))==0 ) return 1.0    ;
  if ( strncmp("km"  ,u,strlen(u))==0 ) return 1.0e-03;
  if ( strncmp("nm"  ,u,strlen(u))==0 ) return 1.0e-09;
  if ( strncmp("pm"  ,u,strlen(u))==0 ) return 1.0e-12;
  if ( strncmp("fm"  ,u,strlen(u))==0 ) return 1.0e-15;
  /* undades de velocidade */
  if ( strncmp("km/s",u,strlen(u))==0 ) return 1.0e+03;
  if ( strncmp("m/s" ,u,strlen(u))==0 ) return 1.0    ;
  if ( strncmp("mm/s",u,strlen(u))==0 ) return 1.0e-03;
  if ( strncmp("nm/s",u,strlen(u))==0 ) return 1.0e-09;
  /* intervalos de tempo */
  if ( strncmp("Ma"  ,u,strlen(u))==0 ) return 3.1536e+13; /* 10^6 anos */
  if ( strncmp("ka"  ,u,strlen(u))==0 ) return 3.1536e+10; /* 10^3 anos */
  if ( strncmp("ano" ,u,strlen(u))==0 ) return 3.1536e+07; /* o ano     */
  if ( strncmp("a"   ,u,strlen(u))==0 ) return 3.1536e+07; /* o ano     */
  if ( strncmp("dia" ,u,strlen(u))==0 ) return 86400     ; /* o dia     */
  if ( strncmp("My"  ,u,strlen(u))==0 ) return 3.1536e+13; /* 10^6 anos */
  if ( strncmp("ky"  ,u,strlen(u))==0 ) return 3.1536e+10; /* 10^3 anos */
  if ( strncmp("year",u,strlen(u))==0 ) return 3.1536e+07; /* o ano     */
  if ( strncmp("y"   ,u,strlen(u))==0 ) return 3.1536e+07; /* o ano     */
  if ( strncmp("day" ,u,strlen(u))==0 ) return 86400     ; /* o dia     */
  if ( strncmp("d"   ,u,strlen(u))==0 ) return 86400     ; /* o dia     */
  if ( strncmp("h"   ,u,strlen(u))==0 ) return 3600      ; /* a hora    */
  if ( strncmp("s"   ,u,strlen(u))==0 ) return 1.0       ; /* o segundo */
  if ( strncmp("ms"  ,u,strlen(u))==0 ) return 1.0e-03   ; /* 10^-3  s  */
  if ( strncmp("ns"  ,u,strlen(u))==0 ) return 1.0e-09   ; /* 10^-9  s  */
  if ( strncmp("ps"  ,u,strlen(u))==0 ) return 1.0e-12   ; /* 10^-12 s  */
  if ( strncmp("fs"  ,u,strlen(u))==0 ) return 1.0e-15   ; /* 10^-15 s  */
  /* unidades de massa */
  if ( strncmp("t"   ,u,strlen(u))==0 ) return 1.00e+03; /* tonelada     */
  if ( strncmp("kg"  ,u,strlen(u))==0 ) return 1.00    ; /* o quilograma */
  if ( strncmp("g"   ,u,strlen(u))==0 ) return 1.00e-03; /* a grama      */
  if ( strncmp("mg"  ,u,strlen(u))==0 ) return 1.00e-06; /* 10^-3 g      */
  /* unidades variadas em astronomia */
  if ( strncmp("R+"  ,u,strlen(u))==0 ) return 6.371e+6;  /* raio da Terra   */
  if ( strncmp("Ro"  ,u,strlen(u))==0 ) return 6.96e+7;   /* raio da Terra   */
  if ( strncmp("AU"  ,u,strlen(u))==0 ) return 1.496e+10; /* unidade astro   */
  if ( strncmp("UA"  ,u,strlen(u))==0 ) return 1.496e+10; /* unidade astro   */
  if ( strncmp("km2" ,u,strlen(u))==0 ) return 1.00e+06;  /* p/ J2*R^2       */
  if ( strncmp("M+"  ,u,strlen(u))==0 ) return 5.98e+24;  /* Massa da Terra  */
  if ( strncmp("Mo"  ,u,strlen(u))==0 ) return 1.99e+30;  /* Massa do Sol    */
  if ( strncmp("deg" ,u,strlen(u))==0 ) return PI/180. ;  /* ângulo em graus */
  if ( strncmp("rad" ,u,strlen(u))==0 ) return 1.0     ;  /* ângulo em rad.  */
  
  printf("\n(newscanf_le_unidade) unidade %s é desconhecida...\n",u);
  return 1.0;
  
}

double newscanf_le_valor(int verbose, int verbose2, char u[], char *string) 
{
  double valor1;
  double valor2;
  valor1 = newscanf_le_numero(verbose,string);
  valor2 = newscanf_le_unidade(verbose,u,1);  
  if ( verbose == 1   ) 
  {
    printf("\n");
    if ( verbose2 == 1 )
      if ( valor2  != 1.0 ) 
	printf("%40s = %+-18.11g SI\n","",valor1 * valor2);
  }
  return valor1 * valor2 ;
}
