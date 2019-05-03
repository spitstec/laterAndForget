

#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>

#define bool unsigned char
#define true 1
#define false 0
#define txtSKIPEXAMP
#define txtPOOLSIZE 16*32768 // available memory for strings
#define txtMAXLEN 4096   // max string length

#include "milkstrict.c"

#define forloop(x,y,z) for (x = (y);x <= (z);x++) 
#define intloop(x,y,z) int x ;for (x = (y);x <= (z);x++) 


FILE * fglob1, *fglob2,*fglob3 ;


bool readstringfrom(tXt fname,tXt * ps ,FILE ** pfi)  {
    if (*pfi == NULL) {
        *pfi = fopen(fname,"r") ; }
    if (*pfi != NULL) {
        char inbuf[txtMAXLEN] ;
        inbuf[0] = 0 ;
        if (fgets(inbuf,txtMAXLEN,*pfi) == NULL)  {
            fclose(*pfi) ;
            *pfi = NULL ; }
        else  {
            int li = strlen(inbuf) ; 
            if (li > 0 && inbuf[li-1]=='\n')  {
                inbuf[li-1]=0 ;  }
            *ps =  txtConcat(inbuf,NULL) ;  }}
    return (*pfi != NULL ) ; }


void writestringto(tXt fname,tXt s , FILE ** pfo)  {
    if (*pfo == NULL) {
        *pfo = fopen(fname,"w+") ; }
    if (*pfo != NULL) {
        if (!s[0]) {
            fprintf(*pfo,"\n") ; }
        else {
            fprintf(*pfo,"%s\n",s) ;  }}}


void finishfile(FILE ** pf)  {
    if (*pf != NULL) {
        fclose(*pf) ; }
    *pf = NULL ;  }

// test file
bool FileExists(tXt nam)  {
    bool rslt = true ;
    FILE * fi = fopen(nam,"r") ;
    if (fi == NULL) {
        rslt = false ; }
    else {
        fclose(fi) ; }
    return rslt ;  }


    

//investigate if char in a range of chars 
bool inset(char c,tXt st)  {
    int i = 0 ;
    bool found = false ;
    while (i < strlen(st) && !found)  {
        found = (c >= st[i] && c <= st[i+1]) ;
        i += 2 ;  }
    return found ; }





// limit an integer
int lim(int x,int mn,int mx) {
  if (x > mx)
    x = mx ;
  if (x < mn)
    x = mn ;
  return x ; }

#define print3int(x,y,z) printf(#x "=%d "#y "=%d "#z "=%d\n",x,y,z) ;
#define EQS(sa,sb) (strcmp(sa,sb)==0)
#define print3string(xa1,xa2,xa3) printf("%s=%s %s=%s %s=%s \n" ,#xa1,xa1 ,#xa2,xa2 ,#xa3,xa3 );

bool commentmode = false ;
bool commenteol = false ;
int commentpos = 999 ;
tXt getctoken(tXt *ps)
{
  int      i;
  tXt res = "" ;
  commenteol = commentmode ;
  if (commentmode) {
    i = txtPos(*ps,"*/") ;
    if (i > 999 )
      i = strlen(*ps)-2 ;
    else {
      commentpos = 9999 ;
      commentmode = false ; }
    commenteol = commentmode ;
    i += 2 ;
    //res = txtSub(*ps,0,i) ;
    res = txtEmpty ;
    *ps = txtDelete(*ps,0,i) ; }
  else {
    while ((*ps)[0] && ((*ps)[0] == ' ' || (*ps)[0] == '\t'))
      (*ps)++ ;
    if ((*ps)[0]) {
      if (EQS(txtSub(*ps,0,2),"//")) {
        res = txtConcat(*ps,NULL) ;
        commenteol = true ;
        *ps = txtEmpty ; }
      if (txtPos(*ps,"/*")==0)  {
        i = txtPos(*ps,"*/");
        if (i <999) {
          i += 2 ;
          res = txtSub(*ps,0,i) ;
          (*ps) += i ; }
        else {
          commentmode = true ;
          res = txtConcat(*ps,NULL) ;
          *ps = txtEmpty ; } }
      if (!res[0]) {
        if (inset((*ps)[0] ,"\"\"\'\'")) {
          res = txtSub(*ps,0,1) ;
          (*ps)++ ;
          i = txtPos(*ps,txtC(res[0])) ;
          if (i < 999)
            res = txtConcat(res,txtSub(*ps,0,i+1),NULL) ;
          (*ps)+= i+1 ; }
        else {
          if (inset((*ps)[0] ,"azAZ09$$__")) {
            i = 1 ;
            if (inset((*ps)[1],"09"))
              while (i < strlen(*ps) && inset((*ps)[i], "09.."))
                i++ ;
            else
              while (i < strlen(*ps) && inset((*ps)[i],"azAZ09__"))
                i++ ;
            res = txtSub(*ps,0,i) ;
            (*ps) += i ; }
          else {
            if (txtPos("/*-+=&%><",txtC((*ps)[0]))<999) {
              i = 1 ;
              if ((strlen(*ps) >=2) && txtPos("/*-+=&%><",txtC((*ps)[1])) <999 ) {
                i = 2 ;
                if ((strlen(*ps) > 2) && ((*ps)[2] == '='))
                  i = 3  ;}
              res = txtSub(*ps,0,i) ;
              (*ps)+=i  ; }
            else {
              res = txtSub(*ps,0,1) ;
              (*ps)++ ; } } }  }  } }
  if (txtPos(res,"/*") == 0 || txtPos(res,"//") == 0 || commenteol || commentmode)
    res = txtEmpty ;
  return res ;
}

tXt LoopMacro = "" ;
tXt EvStartMacro = "" ;

void updateLoopMacro(tXt name,int rise) {
  if (!LoopMacro[0]) 
    LoopMacro = fridge("#define EvLoopMacro") ;
  int p = txtPos(LoopMacro,txtConcat("(",name,")",NULL)) ;
  if (p > 999) {
    LoopMacro = unfridge(LoopMacro) ;
    if (rise == 2)
      LoopMacro = fridge(txtPrintf("%s checkChange(%s) ;",LoopMacro,name)) ;
    else
      LoopMacro = fridge(txtPrintf("%s checkinp%c(%s) ;",LoopMacro,(rise?'h':'l'),name)) ;}
  else {
    char hl = LoopMacro[p-1] ;
    if (hl != ' ' && hl != 'e')
      if ((rise == 1) != (hl == 'h'))
        txtPoke(LoopMacro,p-1,' ') ;}
 if (txtAnyError())
    printf("Err1: %s\n",txtLastError()) ;

}

tXt getext(tXt s) {
  char *p = strstr(s,".") ;
  if (!p)
    return txtEmpty ;
  else {
    ++p ;
    char * p2 = strstr(p,".") ;
    while (p2) {
      p = ++p2 ;
      p2 = strstr(p,".") ; }
    return txtSub(p,0,99) ; }
}
 
tXt unComment(tXt s) {
  int p = txtPos(s,"//") ;
  if (p > 999)
    p = txtPos(s,"/*") ;
  if (p > 999)
    return s ;
  else
    return txtSub(s,0,p-1) ; }
   

tXt prevtoken[8] = {" "," "," "," "," "," "," "} ;

bool tokens (tXt a, tXt b, tXt c,tXt d) {
  int i ;
  tXt abcd[4] = {d,c,b,a} ;
  forloop(i,0,3)
    if (abcd[i][0])
      if (!EQS(abcd[i],prevtoken[i])) {
        i = 999 ; }
  return i < 10 ; }



    

int main(int argc, char * argv[])  {
  tXt finame = "" ;
  tXt foname = "" ;
  int i ;
  int li ;
  tXt s,tkn ;
  EvStartMacro = fridge("#define EvStartMacro") ;
  if (argc >=2 && EQS(getext(argv[1]),"ino")) {
    finame = fridge(argv[1]) ;
    foname = fridge("harvested.h") ; }
  else {
    forloop(i,1,argc-1) 
      if (EQS(getext(argv[i]),"cpp")) 
        if (txtPos(argv[i],"\\sketch\\") < 999)
          finame = fridge(txtPrintf("%s",argv[i])) ;
    if (!finame[0]) {
      printf("no valid cpp file in arguments\n") ;
      return 0 ; }
    foname = txtFlip(finame) ;
    txtEat(&foname,'\\') ;
    foname = fridge(txtConcat(txtFlip(foname),"\\harvested.h",NULL)) ;}
  print3string(foname,finame," ") ;
  while (  readstringfrom(finame,&s,&fglob1) ) {
    if (txtPos(s,"//gBl") == 0)
      writestringto(foname,txtSub(s,5,999),&fglob2) ;
    int rep = 0 ;
    while (s[0] ) {
      tXt tkn = getctoken(&s) ;
      if (tkn[0]) {
        intloop(i,0,6)
          prevtoken[7-i] = prevtoken[6-i] ;
        prevtoken[0] = txtConcat(tkn,NULL) ;
        if (!tokens("#","define","","")) {
            if (txtPos(prevtoken[1], "glob_") == 0 && isalpha(tkn[0])) {
              //printf("%s %s %s\n",prevtoken[2],prevtoken[1],tkn) ;
              writestringto(foname,txtConcat(txtSub(prevtoken[1],5,99)," ",tkn, " ;",NULL),&fglob2) ;} }
        if (!EQS(prevtoken[4],"define") ) {
          if (tokens("onRise","(","", ")")) 
            updateLoopMacro(prevtoken[1],1) ;
          if (tokens("onFall","(","", ")")) 
            updateLoopMacro(prevtoken[1],0) ;
          if (tokens("onChange","(","",")"))
            updateLoopMacro(prevtoken[1],2) ; }
        if (EQS(tkn,"addSetup")  && !tokens("","#","define","") ) {
          EvStartMacro = fridge(txtConcat(unfridge(EvStartMacro),unComment(s),NULL)) ; } } } }
  if (EvStartMacro[0])   
    writestringto(foname,unfridge(EvStartMacro),&fglob2) ;
  if (LoopMacro[0])
    writestringto(foname,unfridge(LoopMacro),&fglob2) ;
  finishfile(&fglob2) ;
  if (txtAnyError())
    printf("Err: %s\n",txtLastError()) ;
  return 1 ;
}  
