//-----------------------------------------------------------------------------
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
//-----------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
typedef long double real;
//------------------------------------------------------------------------------
int binaryNum[5];
//------------------------------------------------------------------------------
class clEvent{
   public:
      int  nq;
      real iat, st, at, bs, es, idt;
};
//------------------------------------------------------------------------------
class clQS{
   public:
      real l, m, T, U, E[7], V[7];
      vector<clEvent> Event;
};
//------------------------------------------------------------------------------
class clQSN{
   private:
      size_t s;
      vector<size_t> S;
      real F     ( real );
      void Open  ( size_t, size_t );
      void Close ( size_t, size_t );
      void Nq    ( void );

   public:
      vector<clQS> QS;

      void Config   ( int, vector<real>, int, vector<size_t> );
      void Simulate ( int );
      void Statistic( int );
};
//------------------------------------------------------------------------------
void clQSN::Config( int qs, vector<real> D, int s, vector<size_t> S ){
    this->s = s;
    this->S = S;
    QS.clear();
    for( int i = 0; i < qs; i++ ){
         clQS x;
         x.l = 0.0;
         if( i == 0 ) x.l = D[i];
         x.m = D[i+1];
         QS.push_back(x);
    }
}
real clQSN::F( real p ){
   real u = (rand()+1.0)/(RAND_MAX+2.0); // u in (0,1)
   return -p*log(u);
}
void clQSN::Open( size_t i, size_t f ){
     clEvent X, Xa;
     Xa = QS[i].Event[ QS[i].Event.size()-1 ];
     if( i == 0 ){
         X.iat = F( QS[i].l );
         X.st  = F( QS[i].m );
         X.at  = Xa.at + X.iat;
         X.bs  = X.at > Xa.es ? X.at : Xa.es;
         X.es  = X.bs + X.st;
         X.idt = X.es - Xa.es;
         QS[i].Event.push_back(X);
//         Nq(i);
     }
     Xa    = QS[i].Event[ QS[i].Event.size()-1 ];
     X.iat = Xa.idt;
     X.st  = F( QS[f].m );

     Xa    = QS[f].Event[ QS[f].Event.size()-1 ];
     X.at  = Xa.at + X.iat;
     X.bs  = X.at > Xa.es ? X.at : Xa.es;
     X.es  = X.bs + X.st;
     X.idt = X.es - Xa.es;
     QS[f].Event.push_back(X);
//     Nq(f);
}
void clQSN::Close( size_t i, size_t f ){
     clEvent X, Xa;

     Xa    = QS[i].Event[ QS[i].Event.size()-1 ];
     X.iat = Xa.idt;
     X.st  = F( QS[f].m );

     Xa = QS[f].Event[ QS[f].Event.size()-1 ];
     X.at  = Xa.at + X.iat;
     X.bs  = X.at > Xa.es ? X.at : Xa.es;
     X.es  = X.bs + X.st;
     X.idt = X.es - Xa.es;
     QS[f].Event.push_back(X);
//     Nq(f);
}
void clQSN::Nq( void ){
   for( size_t qs = 0; qs < QS.size(); qs++ ){
        for( size_t e = 1; e < QS[qs].Event.size(); e++ ){
             size_t c = e-1;
             QS[qs].Event[e].nq = 0;
             while( QS[qs].Event[e].at < QS[qs].Event[c].es ){
                    QS[qs].Event[e].nq += 1;
                    c--;
             }
        }
   }
}
void clQSN::Simulate( int N ){
    for( size_t i = 0; i < s; i++ )
        QS[ S[i] ].Event.push_back({0,0,0,0,0,0,0});
    for( int e = 1; e < N; e++ ){
        for( size_t i = 1; i < s; i++ ){
             Open(S[i-1],S[i]);
            //Close(S[i-1],S[i]);
        }
   }
}
void clQSN::Statistic( int Ni ){
    Nq();
    for( size_t qs = 0; qs < QS.size(); qs++ ){
        size_t N = QS[qs].Event.size();
        real   x[7], Sx[7], Sxx[7];
        for( size_t s = 0; s < 7; s++ )
             Sx[s] = Sxx[s] = 0.0;
        for( size_t e = Ni; e < N; e++ ){
             clEvent X = QS[qs].Event[e];
             x[0] = X.iat    ;
             x[1] = X.st     ;
             x[2] = X.nq     ;
             x[3] = X.idt    ;
             x[4] = X.bs-X.at; //w
             x[5] = X.es-X.bs; //s
             x[6] = X.es-X.at; //r
             for( int s = 0; s < 7; s++ ){
                  Sx [s] += x[s];
                  Sxx[s] += x[s]*x[s];
             }
        }
        QS[qs].T = QS[qs].Event[N-1].es-QS[qs].Event[Ni-1].bs;
        QS[qs].U = Sx[1]/QS[qs].T;
        for( int s = 0; s < 7; s++ ){
             QS[qs].E[s] = Sx [s]/(N-Ni);
             QS[qs].V[s] = Sxx[s]/(N-Ni)-QS[qs].E[s]*QS[qs].E[s];
        }
   }
}
void decToBinary(int n) 
{ 
    int i = 0; 
    while (n > 0) { 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    }
} 
string RI( void ){
    int R  = 10,
        N  = 1000,
        Ni = 0.9*N,
        qs = 5,
        s  = 12;
    vector<size_t> S = { 0, 1, 3, 1, 2, 1, 4, 1, 2, 4, 1, 0 };
    vector<real>   S1 = {0.1, 0.2, 0.2, 0.1, 0.4};
    vector<real>   S2 = {1, 2, 2, 1, 4};
    vector<real>   D = { 0.0288, 0.0027, 0.002, 0.005, 0.0222, 0.0284 };

    clQSN QSN[R];

    for ( int i = 0; i < qs*qs; i++ ) {
        stringstream str;

        str << fixed;
        str.precision(3);
        cout << " Iteração: " << i << endl;
        decToBinary(i);
        for ( int j = qs; j > 0; j-- ) {
            D[5-j+1] = binaryNum[5-j] == 0 ? S2[5-j] : S1[5-j];
        }

        for( int r = 0; r < R; r++ ){
                cout << " Calculando RI: " << r << endl;
                srand( time(NULL)/(r+1) );
                QSN[r].Config(qs,D,s,S);
                QSN[r].Simulate(N);
                QSN[r].Statistic(Ni);
        }

        for( int q = 0; q < qs; q++ ){
            str << "QS,"
                << "T,"
                << "E[nq],"
                << "E[w],"
                << "U,";
        }
        str << "\n";

        for( int r = 0; r < R; r++ ){
            str << r+1 << ",";
            for( size_t qs = 0; qs < QSN[r].QS.size(); qs++ ){
                str << qs << ","
                    << QSN[r].QS[qs].T << ","
                    << QSN[r].QS[qs].E[2] << ","
                    << QSN[r].QS[qs].E[4] << ","
                    << QSN[r].QS[qs].U << ",";
            }
            str << "\n";
        }

        ofstream fo;
        string file = "QSN-RI-rep" + std::to_string(i) + ".csv";
        fo.open(file);
        fo << str.str();
        fo.close();
        // return str.str();
    }

}
int main( void ){
    RI();
    return 0;
}
//------------------------------------------------------------------------------