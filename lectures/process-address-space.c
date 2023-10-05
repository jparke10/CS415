#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int	a, b, c;

int r(int d, int e, int f, int *z)
{
  int	i;
  float	g;

  printf("\nSTACK\n");
  printf("Address of d = %p, e = %p, f = %p\n", &d, &e, &f);
  printf("Address of i = %p, g = %p\n", &i, &g);

  g = d*10.0 + e*10.0 + f*10.0;

  printf("\nHEAP\n");
  for (i=0; i<10; i++){
    z[i] = i;
    printf("Address of z[%d] = %p\n", i, &z[i]);
  }

  return 0;
} /* r */

int main(int argc, char *argv[])
{
  int	*array;
  pid_t	pid;

  pid = getpid();
  printf("PID: %d\n", pid);

  printf("\nSTACK\n");
  printf("Address of argc = %p, argv = %p\n", &argc, &argv);
  printf("Address of array = %p\n", &array);
  printf("Address of pid = %p\n", &pid);

  printf("\nGLOBAL VARIABLES\n");
  printf("Address of a = %p, b = %p, c = %p\n", &a, &b, &c);

  a = 1; b=2; c=3;
  array = malloc(sizeof(int)*10);

  printf("\nHEAP\n");
  printf("Address of array[] = %p\n", array);

  r(a, b, c, array);

  printf("PID %d sleeping\n", pid);
  sleep(30);
  printf("PID %d waking and exiting\n", pid);
  return 0;

} /* main */

