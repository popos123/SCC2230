#include <Arduino.h>
#include <math.h>
#include <Chrono.h>

double f(double x);
double trapezoidal(double f(double x), double a, double b, int32_t n);
double roundoff(double value, unsigned char prec);

double b = 0;

Chrono count;

void setup() {
  Serial.begin(115200);
}

void loop() {
    if (count.hasPassed(500)) {
    count.restart();
    int32_t i=10000;
    double a=0,integral_new;
    integral_new = trapezoidal(f,a,b,i);
    //integral_new = roundoff(integral_new, 7);
    Serial.println(integral_new, 3);
    b++;
  }
}

double roundoff(double value, unsigned char prec) {
  double pow_10 = pow(10.0f, (double)prec);
  return round(value * pow_10) / pow_10;
}

/* Define the function to be integrated here: */
double f(double x) {
  return x;
}

/*Function definition to perform integration by Trapezoidal Rule */
double trapezoidal(double f(double x), double a, double b, int32_t n) {
  double x,h,sum=0,integral;
  int32_t i;
  h=fabs(b-a)/n;
  for(i=1;i<n;i++){
    x=a+i*h;
    sum=sum+f(x);
  }
  integral=(h/2)*(f(a)+f(b)+2*sum);
  return integral;
}
