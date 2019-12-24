#ifndef scanfuck

char* scanfuck(){ //gets char* of 'infinite' length | TODO: make a cap, because you could crash a system with a 9gb string. Could literally use fgets()
  int len_max = 10;
  int current_size = len_max;
  
  char* pStr = malloc(len_max);
  
  if (pStr != NULL) {
    int c = EOF;
    int i = 0;
    while ((c = fgetc(stdin)) != '\n') {
      pStr[i++] = (char)c;
      if (i == current_size){
	current_size = i + 1; //i++ wouldnt work for some reason
	pStr = realloc(pStr, current_size);
      }
    }
    pStr[i] = '\0';
  }
  return pStr;
  
}

#endif
