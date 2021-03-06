/* head.c
   HW #1 - Operating Systems - CS-GY 6233
   Spring 2019 - NYU Bridge to Tandon
   J. Michael Ruby and Daisy B. Crego

   n.b.: Additional behaviors have been added, in emulation of Linux
   functionality, that were not explicitly asked in assignment.

   When no arguments are passed to 'head':
     reads standard input
   When the argument '-[null terminator]' is passed to 'head':
     reads standard input
   When '-0' is passed to 'head':
      error message 'head: illegal line count'
   When '-[non-positive/non-numeric argument]' is passed to 'head':
     error message 'head - invalid option --'[non-positive/non-numeric argument]'
   When multiple files (or instances of standard input) are passed to 'head'
     prints each fd
     preceded by ==> FILENAME (or STANDARD INPUT) <== for first fd
     and '\n==> FILENAME (or STANDARD INPUT) <== for subsequent fd
   When a bad argument is passed to 'head' (not an fd)
     error message

  In order to accomodate large standard input (>512 characters), dynamic memory
  allocation is used. In all other cases (e.g. file input), dynamic memory
  allocation is not used.
 */

#include "types.h"
#include "user.h"

#define DEFAULT_LINES 10
char buf[512];

/* Doubles the capacity of inArr. Returns a pointer to the new array, outArr.*/
char*
doubleSize(char* inArr, int size){
    int newSize = size * 2;
    char* outArr = malloc(newSize);
    int i;
    for (i = 0; i < size; i++)
      outArr[i] = inArr[i];
    free(inArr);
    return outArr;
}

/* Prints the contents of arr. No separation between the characters printed. */
void
printArr(char* arr, int size){
  int i;
  for(i = 0; i < size; i++){
    printf(1, "%c", arr[i]);
  }
  return;
}

/* Reads first {lines} of file descriptor, whether file or stdin */
void
head(int fd, int lines, char *name)
{
  if (fd == 0){
    int l = 0;
    int n;
    char temp;
    int bufCap = 512;
    int bufSize = 0;
    char* dynamicBuf = malloc(bufCap);
    while((l < lines) && ((n = read(fd, &temp, 1)) > 0)){
      dynamicBuf[bufSize] = temp;
      bufSize++;
      if (bufSize == bufCap){
        dynamicBuf = doubleSize(dynamicBuf, bufSize);
        bufCap *= 2;
      }
      if (temp == '\n'){
        l++;
        printArr(dynamicBuf, bufSize);
        bufSize = 0;
      }
    }
    free(dynamicBuf);
    return;
  }

  else{
    int n;
    int l = 0;
    int i;
    while((n = read(fd, buf, sizeof(buf))) > 0) {
      for(i = 0; i < n; i++) {
        if(l < lines) {
          printf(1, "%c", buf[i]);
          if(buf[i] == '\n'){
            l++;
            if((l == lines))
              return;
          }
        }
      }
    }
    return;
  }
}

/*
Return values:
0   Option is for line number. In this case, lines is updated with
the number of lines.
1   Option is illegal (negative or non-numeric character).
2   Option is an invalid line count (0).
3   Option is for standard input ('-').
4   Any other case. Option may be a file name.

If lines is not explicitly set, it will be set to DEFAULT_LINES.
*/
int
parseOption(char* argToParse, int* lines){
  *lines = DEFAULT_LINES;
  char *arg = argToParse;
  if (*(arg) == '\0'){
    return 3;
  }
  else if(*(arg) >= 49 && *(arg) <= 57){
    *lines = atoi(arg);
    return 0;
  }
  else if (*(arg) == 48){
    arg++;
    if(*(arg) == '\0'){
      return 2;
    }
    else{
      while (*(arg) == 48){
        arg++;
      }
      if (*(arg) == '\0'){
        return 2;
      }
      else {
        if(*(arg) >= 49 && *(arg) <= 57){
          *lines = atoi(arg);
          return 0;
        }
      }
    }
  }
  else
    return 1;
  return 4;
}


int main(int argc, char *argv[])
{
  int lines, line_arg_offset, fd, fn_index, optionUsed;
  line_arg_offset = 0;
  lines = DEFAULT_LINES;

  if (argv[1][0] == '-'){
    char* arg = argv[1];
    arg++;
    optionUsed = parseOption(arg, &lines);
    switch(optionUsed){
      case 0:
        line_arg_offset = 1;
        break;
      case 1:
        printf(2, "head: invalid option -- '%c'\n", argv[1][1]);
        exit();
      case 2:
        printf(2, "head: illegal line count\n");
        exit();
      case 3:
        fd = 0;
        line_arg_offset = 1;
        break;
      case 4:
        line_arg_offset = 0;
        break;
    }
  }

  fn_index = 1 + line_arg_offset;

  if ((argc <= 1) || ((argc == 2) && (line_arg_offset == 1))) {
    head(0, lines, "");
  }
  else {
    int i;
    for(i = fn_index; i < argc; i++) {
      if(argv[i][0] == '-') {
        char *arg = argv[i];
        arg++;
        if (*arg == '\0'){
          if(argc - fn_index > 1) {
            if(i - fn_index > 0)
              printf(1, "\n");
            printf(1, "==> standard input ==>\n");
          }
          head(0, lines, "");
        }
        else{
          printf(2, "head: invalid option -- '%c'\n", argv[i][1]);
          exit();
        }
      }
      else {
        if((fd = open(argv[i], 0)) < 0) {
          printf(2, "head: cannot open '%s' for reading\n", argv[i]);
          if(i + 1 == argc)
            exit();
        }
        else {
          if(argc - fn_index > 1) {
            if(i - fn_index > 0)
              printf(1, "\n");
            printf(1, "==> %s <==\n", argv[i]);
          }
        }
      }
      head(fd, lines, argv[i]);
      close(fd);
      exit();
    }
  }
}
