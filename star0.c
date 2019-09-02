#include <stdio.h>
#include <math.h>

#define GG    6.67e-11     // constante de gravitação universal
#define ANO   3.1556736e7  // extensão do ano em segundos - Se não colocar em exponencial, limite de 65 anos
#define DIA   8.6400e4     // extensão do dia em segundos

#define NMAX 10            // número máximo de corpos
#define NRE   1            // número de dimensões (3D neste caso)
#define NDIM  3            // como em C a matriz começa em 0 , some 1
			               // caso seja em fortran some 0	

int main(){

int    i            ; // ponteiro para a i-ésimo componente
int    j            ; // ponteiro para a j-ésimo componente
int    k            ; // ponteiro para a k-ésimo componente
int    N            ; // número de corpos

double t_sim        ; // [ s  ] instante no tempo
double t            ; // [ s  ] instante no tempo
double dt           ; // [ s  ] passo de tempo
double dr[NDIM+NRE] ; // [ m  ] vetor posição relativa
double dr2 			; // [ m² ] quadrado do módulo deste
double dr1 			; // [ m  ] módulo deste
double ds[NDIM+NRE] ; // [ m  ] vetor posição relativa
double ds2 			; // [ m² ] quadrado do módulo deste
double ds1 			; // [ m  ] módulo deste
double dsr 			; // [ m  ] módulo deste
double aux 			; // [ ** ] variável auxiliar 

double RCM[NDIM+NRE];
double VCM[NDIM+NRE];
double MCM          ;


// Descrição geométrica do movimento
//      [corpo]   [componente]
double r[NMAX+NRE][NDIM+NRE]    ; // [ n  ] vetor posição
double v[NMAX+NRE][NDIM+NRE]    ; // [ n  ] vetor velocidade
double a[NMAX+NRE][NDIM+NRE]    ; // [ n  ] vetor aceleração
//Verlet
double r_a[NMAX+NRE][NDIM+NRE]    ; // [ n  ] vetor posição anterior
double r_d                        ; // [ n  ] vetor posição depois (tmp) 

double m[NMAX+NRE]              ; // [ kg ] massa

// parâmetros 

dt    = 0.1*DIA     ; // passo de integração
t_sim = 1*ANO ; // 
N     = 3       ; // dois corpos

// condições iniciais
m[1]    =   1.99e30;
r[1][1] =   0.0    ; // 1-corpo:Sol
r[1][2] =   0.0    ; 
r[1][3] =   0.0    ; 
v[1][1] =   0.0    ; 
v[1][2] =   0.0    ; 
v[1][3] =   0.0    ; 

m[2]    =   5.98e24;
r[2][1] = 150.0e9  ; // 2-corpo:Terra
r[2][2] =   0.0    ; 
r[2][3] =   0.0    ;  
v[2][1] =   0.0    ; 
v[2][2] =  30.0e3  ; 
v[2][3] =   0.0    ; 

m[3]    =   7.342e22;
r[3][1] = 150.0e9 + 3.85e8  ; // 3-corpo:Lua
r[3][2] =   0.0    ; 
r[3][3] =   0.0    ;  
v[3][1] =   0.0    ; 
v[3][2] =  30.0e3 + 1022 ; 
v[3][3] =  89.0    ; 

// Cálculo do centro de massa para tornar o sistema estático:
// quantidade de movimento inicial zero!

for ( i=1 ; i<=N ; i++ ){
		MCM    = 0.0 ;  
	for ( k=1 ; k<=NDIM ; k++ ) {
		RCM[k] = 0.0 ; // para ter certeza que o valor inicial é zero
		VCM[k] = 0.0 ;
	}
}

for ( i=1 ; i<=N ; i++ ){
		MCM	   += m[i];
	for ( k=1 ; k<=NDIM ; k++ ) {
		RCM[k] += m[i] * r[i][k]; // calculando efetivamente
		VCM[k] += m[i] * v[i][k];
	}
}

for ( i=1 ; i<=N ; i++ ){
	for ( k=1 ; k<=NDIM ; k++ ) {
		r[i][k] = r[i][k] - RCM[k]/MCM; // média ponderada
		v[i][k] = v[i][k] - VCM[k]/MCM;
	}
}


// inicialização do método de Verlet
for ( i=1 ; i<=N ; i++ ){
	for ( k=1 ; k<=NDIM ; k++ ) r_a[i][k] = r[i][k] -  v[i][k] * dt;
}

for ( t=0; t<=t_sim ; t+=dt ){

	// saída de dados do instante t
	// for ( i=1 ; i<=N ; i++ ){
	// printf("%g ", t); // [ s ] instante do tempo $1
	// printf("%d ", i); // [ n ] número do corpo   $2
	// for ( k=1 ; k<=NDIM ; k++ ) printf("%g ",r[i][k]); // [ m ] posição    $3 4 5 
	// for ( k=1 ; k<=NDIM ; k++ ) printf("%g ",v[i][k]); // [m/s] velocidade $6 7 8
	// printf("\n"); // fim de linha
	// }

	for ( k=1, ds2=0 ; k<=NDIM ; k++ ){
		ds[k] = r[1][k] - r[2][k] ;   // vetor diferença de posição
		ds2  +=   ds[k] *   ds[k] ;   // será o quadrado do módulo desta
	}  // dr2 = dr2 + dr[k]*dr[k]
			ds1 = sqrt(ds2) ;                 // módulo da diferença de posição

	for ( k=1, dr2=0 ; k<=NDIM ; k++ ){
		dr[k] = r[3][k] - r[2][k] ;   // vetor diferença de posição
		dr2  +=   dr[k] *   dr[k] ;   // será o quadrado do módulo desta
	}  // dr2 = dr2 + dr[k]*dr[k]
	dr1 = sqrt(dr2) ;                 // módulo da diferença de posição

	for ( k=1, dsr=0.0 ; k<=NDIM ; k++ )	dsr += ds[k] * dr[k]; // produto interno 
		aux = dsr/(ds1*dr1);
	printf("%g %g \n",t, (180.*acos(aux)/M_PI));

// plot 'aula4.dat' u ($2==2?$1:1/0):(sqrt($3*$3+$4*$4)) with points pt 7 ps 0.3, '' u ($2==3?$1:1/0):(sqrt($3*$3+$4*$4))
// plot 'aula4.dat' u ($2==2?$1:1/0):(sqrt($6*$6+$7*$7)) with points pt 7 ps 0.3, '' u ($2==3?$1:1/0):(sqrt($6*$6+$7*$7))

	// Zerando as acelerações
	for ( i=1 ; i<=N ; i++ ) 
		for ( k=1 ; k<=NDIM ; k++ )
			a[i][k] = 0.0 ;

// Cálculo de aceleração para 2 corpos

	for ( i=1 ; i<N ; i++ ){
		for ( j=i+1 ; j<=N ; j++ ){

			for ( k=1, dr2=0 ; k<=NDIM ; k++ ){
				dr[k] = r[j][k] - r[i][k] ;   // vetor diferença de posição
				dr2  +=   dr[k] *   dr[k] ;   // será o quadrado do módulo desta
			}  // dr2 = dr2 + dr[k]*dr[k]
			dr1 = sqrt(dr2) ;                 // módulo da diferença de posição

			aux = GG / (dr2 * dr1) ;          // parte comum

			for ( k=1 ; k<=NDIM ; k++ ){
				a[j][k] -= aux * m[i] * dr[k] ; // k-aceleração gravitacional
				a[i][k] += aux * m[j] * dr[k] ; // k-aceleração gravitacional
			}
		}
	}

	// Integrador: Método de Euler
	// for ( k=1 ; k<=NDIM ; k++ ){
	// 	r[2][k] = r[2][k] + v[2][k] * dt ; 
	// 	v[2][k] = v[2][k] + a[2][k] * dt ; 
	// }

	// Integrador: Método de Verlet
	for ( i=1 ; i<=N ; i++ ){
		for ( k=1 ; k<=NDIM ; k++ ){
		r_d       = 2 * r[i][k] - r_a[i][k] + a[i][k] * dt * dt ;
		v[i][k]   = (r_d - r[i][k]) / dt ;
		r_a[i][k] = r[i][k] ;
		r[i][k]   = r_d ;
		} // end for k
	}// end for i
} // end for t





return 0 ;

}
