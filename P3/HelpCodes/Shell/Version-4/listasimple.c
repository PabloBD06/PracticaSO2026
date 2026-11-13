#include "listasimple.h"


int AniadirElemento (LISTASIMPLE l, void *el) 
{
   int i;
   if (el==NULL){
        errno=EFAULT;
        return -1;
   }
   for (i=0; l[i]!=NULL; i++);
   if (i==MAXLISTASIMPLE-1)
	 {errno=ENOSPC; return -1;}
   l[i]=el;
   l[++i]=NULL;
   return i;
}

void ImprimirListaFirst (LISTASIMPLE l, int cuantos, 
         int n,void (*ImprimirElemento)(void*)) 
                 /*imprime los primeros n elementos*/
{
   int i;
   for (i=0; l[i]!=NULL && i<cuantos; i++){
         if (n) printf ("%d->",i); /*si orden imprime tambien el numero*/
	     (*ImprimirElemento) (l[i]);
   }
}

int ElementosEnLista (LISTASIMPLE l)
{
     int i;
     
    for (i=0; l[i]!=NULL; i++)
       if (i==MAXLISTASIMPLE-1)
	       return MAXLISTASIMPLE;
	return i;
}

void ImprimirListaLast (LISTASIMPLE l, int cuantos,
        int n,void (*ImprimirElemento)(void*))
{                       /*imprime los primeros n elementos*/
   int i=0, num=ElementosEnLista(l);
   
   if ((i=num-cuantos)<0)
        i=0;
   for (; l[i]!=NULL; i++){
         if (n) printf ("%d->",i); /*si orden imprime tambien el numero*/
	     (*ImprimirElemento)(l[i]);
   }
}   

void ImprimirListaCompleta (LISTASIMPLE l, int orden,void (*ImprimirElemento)(void*))
{
      ImprimirListaFirst(l,MAXLISTASIMPLE,orden,ImprimirElemento);
}

void* GetElementoAtPos (LISTASIMPLE l, int n)
{
   int i;
   
   if (n<0)
    return NULL;
   
   for (i=0; i<n; i++)     /*si no hay n*/
       if (l[i]==NULL)
            return NULL;

   return l[n];
}
int BorrarElementoAtPos (LISTASIMPLE l, int pos)
{
   int i;
     if (GetElementoAtPos(l,pos)!=NULL){
        free (l[pos]);
        for (i=pos; l[i]!=NULL;i++)
            l[i]=l[i+1];
        return 0;
        }
     return -1;
}

int BuscarElemento (LISTASIMPLE l, void * el, int (*comp)(void*,void*))
{                               /*pasamos la funcion de comparacion*/
    int i;
    
        if (el==NULL){
        errno=EFAULT;
        return -1;
    }
    for (i=0; l[i]!=NULL; i++)
        if (!(*comp)(l[i],el))
            return i;   /*si lo encotramos devolvemos donde*/
            
    if (i==MAXLISTASIMPLE -1){
        errno=ENOSPC;
        return -1;
        }
    return -1;
}

int BorrarElemento (LISTASIMPLE l, void * el,int (*comp)(void*,void*))
{
    int pos,i;
    
    if ((pos=BuscarElemento(l,el,comp))==-1){
        errno=ENOENT;
        return -1;
        }
    free (l[pos]);
    for (i=pos; l[i]!=NULL;i++)
        l[i]=l[i+1];
    return 0;
}
void BorrarLista(LISTASIMPLE l)
{
   int i;
   for (i=0; l[i]!=NULL; i++){
	free(l[i]);
	l[i]=NULL;
	}
}

void *GetSiguienteElementoAdd (LISTASIMPLE l, void * el, int (*comp)(void *,void*))
{
    int pos;
    
    
    if ((pos=BuscarElemento(l,el,comp))==-1){
        errno=ENOENT;
        return NULL;
        }
    return (l[pos+1]);
}


void *GetFirstNextElement(LISTASIMPLE l, int first)
{
   static int cual=0;
   
   if (first==1)
    cual=0;
    
   return (l[cual++]);
}
void * GetPrimerElemento(LISTASIMPLE l) 
{
     return GetFirstNextElement(l,1);
}

void *GetSiguienteElemento(LISTASIMPLE l)
{
      return GetFirstNextElement(l,0);
}





