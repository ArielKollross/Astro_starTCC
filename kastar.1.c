#include <stdio.h>
#include <string.h>
#include <math.h>

#include "newscanf.c"

#define G 6.67e-11          // constante da gravitação universal
#define ANO 3.1556736e7     // extensão do ano em seg
#define DIA 8.6400e4        // extensao do dia em seg
#define AU 149597870700     // [ m ] unidade astronômica
#define parsec 3.086e16     // [ m ] parsec
#define aluz 9.461e15       // [ m ] ano-luz
#define TRAD (M_PI / 180.0) // conversão de graus para radianos
#define NMAX 8              // numero de corpos
#define NDIM 3              // Dimensões espaciais

//====================================================================================

int main(int argc, char *argv[])
{

  int i, j, k;
  int aux;                      // [  ]
  double t;                     // [  ]
  double t_sim;                 // [  ]
  double dt;                    // [  ]
  double mass[NMAX + 1];        // [  ]massa da estrela
  double r[NMAX + 1][NDIM + 1]; // [  ]posição no espaço cartesiano (x,y,z)
  double v[NMAX + 1][NDIM + 1]; // [  ]velocidade em três componentes (Vx,Vy,Vz)
  double a[NMAX + 1][NDIM + 1]; // [  ]aceleração em 3D(x,y,z)
  //-----------------------------------------------------------------------------------
  //tranformações da versão s/ gravitação

  double alpha;        // [  ] Coord Equatorial
  double delta;        // [  ]Coord Equatorial
  double h, m, n;      // [  ]Constantes auxiliares dos angulos
  double l, l2, b;     // [  ]Coord Galática
  double dist_m;       // [  ]distância de parsec para km
  double Rs[NMAX + 1]; // [  ]modulo da distância estrela-sol
  double x;
  //--------------------------------------------------------------------------------
  // Novos parametros para gravitação

  double r_a[NMAX + 1][NDIM + 1]; // [  ]vetor posição anterior
  double aux2;                    // [  ] para G
  double aux3;                    // [  ] para dsr
  double dr[NDIM + 1];            // [  ]vetor posição relativa
  double dr1;                     // [  ]módulo de dr[]
  double dr2;                     // [  ]quadrado do módulo de dr[]
  double r_d;                     // [  ]vetor posição depois (tempo)
  double ds[NDIM + 1];            // [  ] vetor posição relativa
  double ds2;                     // [  ] quadrado do módulo deste
  double ds1;                     // [  ] módulo deste
  double dsr;                     // [  ] módulo deste

  //--------------------------------------------------------------------------------
  // Parametros do centro de massa
  double RCM[NDIM + 1]; // [  ]
  double VCM[NDIM + 1]; // [  ]
  double MCM;           // [  ]

  // declaração das variavies do Newscanf
  double RAh, RAm, RAs;
  double DEd, DEm;
  double plx;

  dt = ANO * 5e2;
  t_sim = ANO * 1e5;

  //Condições iniciais
  mass[1] = 1.99e30; // Corpo 0 - Sol
  r[1][1] = 0.0;     //[  ]
  r[1][2] = 0.0;
  r[1][3] = 0.0;
  v[1][1] = 0.0; // [  ]
  v[1][2] = 0.0;
  v[1][3] = 0.0;

  // angulos l e b tem de estar em radianos , tranformação -> TRAD

  h = 303.0 * TRAD;  // [rad]
  m = 192.25 * TRAD; // [rad]
  n = 27.4 * TRAD;   // [rad]

  // Importação de Dados pelo Newscanf

  fi = fopen(argv[1], "r");

  for (i = 1; i <= NMAX; i++)
  {
    RAh = newscanf_le_numero(0, "Ascensão reta");             //ascensão reta -> alpha [horas]
    RAm = newscanf_le_numero(0, "Ascensão reta");             //ascensão reta -> alpha [minutos]
    RAs = newscanf_le_numero(0, "Ascensão reta");             //ascensão reta -> alpha [s]
    DEd = newscanf_le_numero(0, "Declinação");                //Declinação -> delta [grau]
    DEm = newscanf_le_numero(0, "Declinação");                //Declinação -> delta [mim]
    plx = newscanf_le_numero(0, "Paralaxe");                  //[mas]-miliseg de arco
    v[i][1] = newscanf_le_numero(0, "Velocidade em x") * 1e3; // C. Galaticas [m/s]
    v[i][2] = newscanf_le_numero(0, "Velocidade em y") * 1e3;
    v[i][3] = newscanf_le_numero(0, "Velocidade em z") * 1e3;
    //mass[i+1][j] = newscanf_le_numero(0,"massa" ) ;

    // passos necessario para negativar as colunas do DEm
    if (DEd < 0)
    {
      DEm = DEm * (-1);
    }

    // Atribuição para cada vetor posição (B1950)

    alpha = (RAh + RAm / 60.0 + RAs / 3600.0) * 15.0 * TRAD; //[rad]

    delta = (DEd + DEm / 60.0) * TRAD; //[rad]

    dist_m = parsec * (1.0 / (plx * 1.0e-3)); //[m]

    x = atan2(sin(192.25 * TRAD - alpha), cos(192.25 * TRAD) * sin(27.4 * TRAD) - (sin(delta) / cos(delta)) * cos(27.4 * TRAD));

    l = 303.0 * TRAD - x; //horizontal

    b = asin(sin(delta) * sin(n) + cos(delta) * cos(n) * cos(m - alpha)); //vertical

    r[i][1] = dist_m * sin(b) * cos(l); //coord galatica
    r[i][2] = dist_m * sin(b) * sin(l);
    r[i][3] = dist_m * cos(b);

  } //end for importação "Newscanf"

  //======================================================================================

  //**** QUE COMEÇE A FÍSICA  :) ****
  //-------------------------------------------------------------------------------------
  // Cálculo do centro de massa para tornar o sistema estático:
  // quantidade de movimento inicial zero!

  for (i = 1; i <= NMAX; i++)
  {
    MCM = 0.0;

    for (k = 1; k <= NDIM; k++)
    {
      RCM[k] = 0.0; // para ter certeza que o valor inicial é zero
      VCM[k] = 0.0;
    }
  }

  for (i = 1; i <= NMAX; i++)
  {
    MCM += mass[i];
    for (k = 1; k <= NDIM; k++)
    {
      RCM[k] += mass[i] * r[i][k]; // calculando efetivamente
      VCM[k] += mass[i] * v[i][k];
    }
  }

  for (i = 1; i <= NMAX; i++)
  {
    for (k = 1; k <= NDIM; k++)
    {
      r[i][k] = r[i][k] - RCM[k] / MCM; // média ponderada
      v[i][k] = v[i][k] - VCM[k] / MCM;
    }
  }

  //--------------------------------------------------------------------------------------
  // INICIALIZAÇÃO DO METODO DE VERLET
  for (i = 1; i <= NMAX; i++)
  {
    for (k = 1; k <= NDIM; k++)
      r_a[i][k] = r[i][k] - v[i][k] * dt;
  } //end for i

  for (t = 0; t <= t_sim; t = t + dt)
  {

    for (i = 1; i <= NMAX; i++)
    {
      // impressão de dados

      //printf("%g ", t       )			   	            ; // $1
      //printf("%d ", i	      )				            ; // $2
      //printf("%g ", Rs[i]/ aluz )			            ; // $3
      //for( k = 1 ; k <= NDIM ; k++) printf("%g ", r[i][k]/ aluz  ); // $4 $5 $6 plot depois set view equal
      //for( k = 1 ; k <= NDIM ; k++) printf("%g ", v[i][k]        ); // $7 $8 $9
      //for( k = 1 ; k <= NDIM ; k++) printf("%g ", densidade      ); // $10 plot "" u 10:1
      //printf("\n ")					            ;
    } // end for i

    //---------------------------------------------------------------------------------------

    for (j = i + 1; j <= NMAX; j++)
    {

      for (k = 1, dr2 = 0; k <= NDIM; k++)
      {
        ds[k] = r[j][k] - r[i][k]; // vetor diferença de posição
        ds2 = ds2 + ds[k] * ds[k]; // modulo quadrado
      }
      ds1 = sqrt(dr2);

      for (k = 1, dr2 = 0; k <= NDIM; k++)
      {

        dr[k] = r[3][k] - r[2][k]; // vetor diferença de posição
        dr2 = dr2 + dr[k] * dr[k]; // será o quadrado do módulo desta
      }
      dr1 = sqrt(dr2); // módulo da diferença de posição
    }                  // end j

    for (k = 1, dsr = 0.0; k <= NDIM; k++)
      dsr += ds[k] * dr[k]; // produto interno
    aux3 = dsr / (ds1 * dr1);

    printf("%g %g \n", t, (180. * acos(aux) / M_PI));
    //----------------------------------------------------------------------------------
    // Zerando as aceleraçõe
    for (i = 1; i <= NMAX; i++) // NÃO ESTA FALTANDO "{}" ?, PQ NAO ESTA?
      for (k = 1; k <= NDIM; k++)
        a[i][k] = 0.0;
    //----------------------------------------------------------------------------------
    // Calculo para acelereção de N-Corpos/ OU ENTRE 2 CORPOS?

    for (i = 1; i < NMAX; i++)
    {
      for (j = i + 1; j <= NMAX; j++)
      {

        for (k = 1, dr2 = 0; k <= NDIM; k++)
        {
          dr[k] = r[j][k] - r[i][k]; // vetor diferença de posição
          dr2 += dr[k] * dr[k];      // será o quadrado do módulo desta
        }                            // end k
        dr1 = sqrt(dr2);             // módulo da diferença de posição

        aux2 = G / (dr2 * dr1);

        for (k = 1; k <= NDIM; k++)
        {
          a[i][j] -= aux2 * mass[i] * dr[k]; // k-aceleração gravitacional
          a[j][k] += aux2 * mass[j] * dr[k]; // k-aceleração gravitacional
        }                                    // end k
      }                                      // end j
    }                                        // Termino do calculo para aceleração de N-corpos- end i
    //=====================================================================================

    // Método de Verlet
    for (i = 1; i <= NMAX; i++)
    {
      for (k = 1; k <= NDIM; k++)
      {
        //r[i][k] = r[i][k] + v[i][k] * dt ; // metodo de Euler

        r_d = 2 * r[i][k] - r_a[i][k] + a[i][k] * dt * dt;
        v[i][k] = (r_d - r[i][k]) / dt;
        r_a[i][k] = r[i][k];
        r[i][k] = r_d;
      } //end k

      Rs[i] = sqrt(r[i][1] * r[i][1] + r[i][2] * r[i][2] + r[i][3] * r[i][3]); //Modulo distância em relação ao Sol(0,0,0)

    } //end for metodo de Verlet

  } // end For do integrador do tempo
  return 0;
} //end main
