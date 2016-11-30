#include <iostream>
#include <math.h>
#include <mpi.h>
#include <time.h>

    double f( double x )
    {
        return (1/(x*x))*(sin(1/x) * sin(1/x));
    }

    int sp = 0;       //указатель вершины стека
    double eps = 0.0000001; //точность вычислений

    struct localStack
    {
        double a;
        double b;
        double fa;
        double fb;
        double sab;
    };

    //макроопределения доступа к данным стека
    #define LOCAL_STACK_IS_NOT_FREE ( mySp != 0 )

    void PUT_INTO_LOCAL_STACK( localStack stack[], int &sp,  double a, double b, double fa, double fb, double sab )
    {
        stack[sp].a = a;
        stack[sp].b = b;
        stack[sp].fa = fa;
        stack[sp].fb = fb;
        stack[sp].sab = sab;
        ++sp;
    }

    void GET_FROM_LOCAL_STACK( localStack stack[], int &sp, double& a, double& b, double& fa, double& fb, double& sab )
    {
        --sp;
        a = stack[sp].a;
        b = stack[sp].b;
        fa = stack[sp].fa;
        fb = stack[sp].fb;
        sab = stack[sp].sab;
    }

    //основная функция
    double IntTrap( double a, double b )
    {
        localStack stack[10000];//массив структур, в которых хранятся отложенные значения. 1000 может быть заменена на необходимое число
        int mySp = 0;
        double I = 0;        //значение интеграла
        double fa = f( a );  //значение функции <math>y = f(x)</math> в точке <math>a</math>
        double fb = f( b );  //значение функции <math>y = f(x)</math> в точке <math>b</math>
        double fc = 0;
        double c = 0;        //середина отрезка интегрирования
        double sab = 0;      //
        double sac = 0;      //
        double scb = 0;      //
        double sabc = 0;     //переменные для подсчёта текущих сумм

        sab = ( fa + fb ) * ( b - a ) / 2;
        while( 1 )
        {
            c = ( a + b ) / 2;
            fc = f( c );
            sac = ( fa + fc ) * ( c - a ) / 2;
            scb = ( fc + fb ) * ( b - c ) / 2;
            sabc = sac + scb;

            if( fabs( sab - sabc ) > eps * fabs( sabc )  ) //<math>\epsilon</math> - заданная точность
            {
                PUT_INTO_LOCAL_STACK( stack, mySp, a, c, fa, fc, sac );
                a = c;
                fa = fc;
                sab = scb;
            }
            else
            {
                I += sabc;

                if( !LOCAL_STACK_IS_NOT_FREE )
                {
                    break;
                }
                GET_FROM_LOCAL_STACK( stack, mySp, a, b, fa, fb, sab );
            }
        }
        return I;
    }

int main( int argc, char** argv )
{
    int rank;
    double diapBA;
    int N;
    const double a = 0.01, b = 1;
    double result = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &N );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    diapBA = ( b - a ) / ( N );
    double I = 0;
    if( 0 == rank )
    {
        dStart = MPI_Wtime();
    }
    I = IntTrap( a + ( rank ) * diapBA, a + ( rank + 1 ) * diapBA );

    MPI_Reduce( &I, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
    if( !rank )
    {
        dFinish = MPI_Wtime();
        printf("number of processes = %d\n", N);
        printf("integral = %f\n", result);
        printf( "time of work = %.5f\n", ( double )(dFinish - dStart) );
    }
    MPI_Finalize();
    return 0;

}
