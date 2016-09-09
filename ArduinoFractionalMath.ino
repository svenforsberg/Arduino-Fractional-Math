#include <stdio.h>
//#define Q 12 //0.0002 Max 8 //3.147 0.787
//#define Q 13 //0.0001 Max 4 //3.136 0.784
#define Q 14 //0.00006 Max 2 //0.785339
//#define Q 15 //0.00003 Max 1 //0.785492

#define Q_MAX ((1<<15)-1)
#define Q_MIN -(1<<15)
#define Q_MAX_L ((1L<<15)-1L)
#define Q_MIN_L -(1L<<15)
#define Q_SCALE_F (float)(1L<<Q)
#define Q_UNITY (1<<Q)

#define MEAN_SHIFT 2
#define MEAN_N (1<<MEAN_SHIFT)

#define _ABS(X) (X ^ (X>>15))-(X>>15)
#define ABS_16(X) (X==-32768 ? (32767) : _ABS(X))
int testdata[16]={1,4,2,4,2,7,34,56,7,8,7,6,4,21,4,9};

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

int old_mean_val=0,oldest_data=0,mean_ct=0;
long mean_val=0;
int mean(int indata,int *utdata) //Only works with one instance.
{
  Serial.print(indata);Serial.print(" ");Serial.print(oldest_data);Serial.print(" ");Serial.println((indata-oldest_data)>>MEAN_SHIFT);
  mean_val=old_mean_val+(indata-oldest_data)>>MEAN_SHIFT;
  old_mean_val=mean_val;
  *utdata=mean_val;
  int result=false;
  if(mean_ct==MEAN_N)
  {
    mean_ct=0;
  }
  else if(mean_ct==MEAN_N-1)
  {
    oldest_data=indata; //Används N steg senare. Måste beräknas för VARJE indata! Varje indata har en oldest_data N steg bakåt. Det kräver N st. lagrade värden...
    Serial.println("True");
    result=true;
  }
  mean_ct++;
//  Serial.println(mean_val);
  return result;
}

/*
N_sum < 65536
Max 32768
*/
int mean2(int indata,int *utdata)
{
  mean_ct++;
  int result=false;

  mean_val+=indata;

  if(mean_ct==MEAN_N)
  {
    mean_val+=1<<(MEAN_SHIFT-1); //Round off
    *utdata=mean_val>>MEAN_SHIFT;
    result=true;
    mean_val=0;
    mean_ct=0;
  }
  
  return result;
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

inline int quick_rms(int data[],int N) //OK. Calcluates average rectified value! 
{
  int i=0;
  long sum=0;
  for(i=0;i<N;i++)
  {
    sum+=ABS_16(data[i]);
  }
  Serial.println(sum);
  return divIntQ0(sum,N);
}

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
  int i;
  float sign=1.0;
  int mean_data,sign_i=1;
  int result=float2Int(1.0);
  float resultf=1.0;

  for(i=0;i<16;i++)
  {
    if( mean2(testdata[i],&mean_data)==true) 
    {
      Serial.print(testdata[i]);Serial.print(" ");Serial.println(mean_data);
    }
  }

/*
  Serial.print("quick_rms=");
  Serial.println(quick_rms(testdata,5));
  Serial.print("RMS_f=");
  Serial.println(int2Float(quick_rms(testdata,5)),7);
*/
  delay(2000);
}






