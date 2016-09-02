#include <stdio.h>
//#define Q 12 //0.0002 Max 8 //3.147 0.787
//#define Q 13 //0.0001 Max 4 //3.136 0.784
//#define Q 14 //0.00006 Max 2 //0.785339
#define Q 15 //0.00003 Max 1 //0.785492

#define Q_MAX ((1<<15)-1)
#define Q_MIN -(1<<15)
#define Q_MAX_L ((1L<<15)-1L)
#define Q_MIN_L -(1L<<15)
#define Q_SCALE_F (float)(1L<<Q)
#define Q_UNITY (1<<Q)


#define ABS_16(X) (X ^ (X>>15))-(X>>15)

unsigned long Timestamp=0;

void setup()
{
  Serial.begin(115200);
}
int signal2dB(int signal)
{
  return (int)(10*log10((float)signal/(float)1023));
}

//Check level: Done with integer math
//Max level: integer math
//Mean level: integer math!
//Square
//Root

int mean_val=0;
int mean_ct=0;
int mean(int in)
{
  mean_ct++;
  mean_val=(mean_val*(mean_ct-1)+in*8)/mean_ct;
  return mean_val/8;
}
void mean_reset()
{
  mean_ct=0;
  mean_val=0;
}

static inline int sat16(long x) //OK!
{
  if(x > Q_MAX_L){
    return Q_MAX; //Overflow at 32767.
  }
  else if( x < Q_MIN_L ){
    return Q_MIN; //Overflow at -32768.
  }
  else{
    return (int)x;
  }
}

static inline int addInt(int a,int b) //OK!
{
  long temp;
  temp=(long)a+(long)b;
  return sat16(temp);
}

static inline int multiplyInt(int a,int b) //OK!
{
  int result;
  long temp;
  temp=(long)a * (long)b;
  temp+=(1L << (Q-1));
  result=sat16(temp >> Q);
  return result;
}

static inline int multiplyIntQ0(int a,int b) //OK
{
  int result;
  long temp;
  temp=(long)a * (long)(b*2);
  temp+=1L;
  result=sat16(temp >> 1);
  return result;
}

static inline int divInt(int a, int b)
{
  int result;
  long temp;
  temp=(long)a << Q;
  if ((temp >= 0 && b >= 0) || (temp < 0 && b < 0))
    temp += b / 2;
  else
    temp -= b / 2;

  result=sat16(temp / b);
  return result;
}

static inline int divIntQ0(long a, long b) //OK!
{
  int result;
  long temp;
  temp=(long)a;
  if ((temp >= 0 && b >= 0) || (temp < 0 && b < 0))
    temp += b / 2;
  else
    temp -= b / 2;

  result=sat16(temp / b);
  return result;
}

void tic_m()
{
  Timestamp=millis();
}

int toc_m()
{
  unsigned long dT=(millis()-Timestamp);
  Serial.print("Elapsed milliseconds=");
  Serial.println(dT);
}

void tic_u()
{
  Timestamp=micros();
}

int toc_u()
{
  unsigned long dT=(micros()-Timestamp);
  Serial.print("Elapsed microseconds=");
  Serial.println(dT);
}
/*
int quick_rms(int data[],int N)
{
  int i;
  for(i=0;i<N;i++)
  {
    ABS_16(data[i]) //abs
  }
}
*/
int float2Int(float x) //OK!
{
  long temp;
  int result;
  if(x>=0) temp=(long)(x*Q_SCALE_F+0.5);
  else temp=(long)(x*Q_SCALE_F-0.5);
  result=sat16(temp);
  return result;
}

float int2Float(int x) //OK!
{
  return ((float)x/Q_SCALE_F);
}

void loop()
{
  long i;
  float sign=1.0;
  int sign_i=1;
  int result=float2Int(1.0);
  float resultf=1.0;
  tic_m();
  for(i=1;i<100000;i++)
  {
    //Float version
    double fraq=1.0F/((double)i*2.0+1.0); //OBS! Talserier är svårt med fixpunkt eftersom kvoterna snabbt blir mycket små!
    sign=-sign;
    resultf+=fraq*sign;
    
    //Integer version
    long den=(i*2L+1L);
    //int fraq_i=divIntQ0(Q_UNITY,den);//Requires Qx on top and Q0 in denominator.
    int fraq_i=float2Int(fraq); //Test
    sign_i=-sign_i;
    //result=addInt(result,fraq_i*sign_i);
    result+=fraq_i*sign_i;
    if( multiplyInt(result,float2Int(4.0))==32767 )
    {
      Serial.print("overflow i=");Serial.println(i);
      break;
    }
  }
  //result=multiplyIntQ0(result,4); //3.1475
  //result=multiplyInt(result,float2Int(4.0)); //3.1475
  //resultf=resultf*4.0; //3.1415
  toc_m();
  Serial.print("Pi_f=");
  Serial.println(resultf,6);
  Serial.print("Pi_q=");
  Serial.print(result);
  Serial.print("=");
  Serial.println(int2Float(result),6);
  


  delay(2000);
}



