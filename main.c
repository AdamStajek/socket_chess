#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*
0-5 - figury gracza
6-11 - figury komputera
12 - puste pole
*/

#define WYGRANA 1000
#define PRZEGRANA -1000

int maksymalny_kierunek[]={8,8,4,4,8,3,8,8,4,4,8,3,0}; 
int liczba_pol[]={2,8,8,8,2,2,2,8,8,8,2,2,0}; //liczba pol o ktore moze przesunac sie figura
int WX[12][8]={{0,1,1,1,0,-1,-1,-1},{0,1,1,1,0,-1,-1,-1},{0,1,0,-1},{1,1,-1,-1},{1,2,2,1,-1,-2,-2,-1},{-1,0,1},{0,1,1,1,0,-1,-1,-1},{0,1,1,1,0,-1,-1,-1},{0,1,0,-1},{1,1,-1,-1},{1,2,2,1,-1,-2,-2,-1},{-1,0,1} };
int WY[12][8]={{-1,-1,0,1,1,1,0,-1},{-1,-1,0,1,1,1,0,-1},{-1,0,1,0},{-1,1,1,-1},{-2,-1,1,2,2,1,-1,-2},{-1,-1,-1},{-1,-1,0,1,1,1,0,-1},{-1,-1,0,1,1,1,0,-1},{-1,0,1,0},{-1,1,1,-1},{-2,-1,1,2,2,1,-1,-2},{1,1,1} };
char buf[10000];
int sock2;

char plansza[8][8]={
8,	11, 12, 12, 12, 12, 5, 2,
10,	11, 12, 12, 12, 12, 5, 4,
9,	11, 12, 12, 12, 12, 5, 3,
6,	11, 12, 12, 12, 12, 5, 1,
7,	11, 12, 12, 12, 12, 5, 0,
9,	11, 12, 12, 12, 12, 5, 3,
10,	11, 12, 12, 12, 12, 5, 4,
8,	11, 12, 12, 12, 12, 5, 2,
};

int ocena();
void wypisz();
int ruch(int tryb, int *x, int *y, int *k, int *o);


int main(){
   int status, sock;

    struct sockaddr_in ser;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock==-1) {
        printf("error socket\n"); return 0;
    }

    ser.sin_family = AF_INET;
    ser.sin_port=htons(4000); //port
    ser.sin_addr.s_addr=inet_addr("127.0.0.23"); //IP

    status = bind(sock, (struct sockaddr *) &ser, sizeof ser);
    if(status==-1){
        printf("error bind\n");
        return 0;
    }

    status = listen(sock, 10);
    if(status != 0){
        printf("error listen\n");
        return 0;
    }


    sock2 = accept(sock, 0, 0);
    if(sock2==-1){
        printf("error accept\n"); return 0;
    }

	int x, y, wyn, k, o, dx, dy, x2, y2;
	while(1==1){
		ruch(4, &x, &y, &k, &o);
		dx=o*WX[plansza[x][y]][k];
		dy=o*WY[plansza[x][y]][k];
		plansza[x+dx][y+dy]=plansza[x][y];
		plansza[x][y]=12;
		if(plansza[x+dx][y+dy]==11 && y+dy==7) plansza[x+dx][y+dy]=7; //promocja pionka na hetmana
		wypisz();
		if(ocena()>=WYGRANA) {printf("Komputer wygral!"); status = send(sock2, buf, strlen(buf), 0); break;}
		else if(ocena()<=PRZEGRANA){printf("Wygrales!"); status = send(sock2, buf, strlen(buf), 0);break;}
		printf("podaj ruch skad dokad");
		scanf("%d%d%d%d", &x, &y, &x2, &y2);
		plansza[x2][y2]=plansza[x][y];
		plansza[x][y]=12;
		if(plansza[x2][y2]==5 && y2==0) plansza[x2][y2]=1; //promocja pionka na hetmana
		wypisz();
		if(ocena()>=WYGRANA) {printf("Komputer wygral!"); status = send(sock2, buf, strlen(buf), 0); break;}
		else if(ocena()<=PRZEGRANA){printf("Wygrales!"); status = send(sock2, buf, strlen(buf), 0);break;}
	}


        close(sock2);
    close(sock);


    return 0;   
}

int ocena(){
	int i, j, w=0;
	int oc[]={PRZEGRANA, -9, -5, -4, -3, -1, WYGRANA, 9, 5, 4, 3, 1, 0};
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
			w+=oc[plansza[i][j]];
	return w;}

void wypisz(){
	char fig[]="khwgspKHWGSP ";
	printf("\n\n\n     y,0  y,1  y,2  y,3  y,4  y,5  y,6  y,7\n");
	printf("    ========================================\n");
	for(int i=0;i<8;i++){\
		printf("%d,x ",i);
		for(int j=0;j<8;j++)
			printf("| %c |", fig[plansza[i][j]]);
		printf("\n    =========================================\n");
	}	}

int ruch(int tryb, int *x, int *y, int *k, int *o){
	int px_pom, py_pom, k_pom, o_pom, px_py, px, py, dx, dy, kierunek, odleglosc;
	int wynik, wmax, wmin, ruch_fig, bita_fig;
	wynik=ocena();
	if(tryb == 0 || 2*wynik>WYGRANA || 2*wynik<PRZEGRANA) return wynik;
	if(tryb%2==0){//komputer
		for(px=0,wmax=100*PRZEGRANA;px<8;px++)
			for(py=0;py<8;py++)
                if(plansza[px][py]>=6 && plansza[px][py]<12) //sprawdzenie czy na polu [px][py] znajduje sie figura komputera
				for(kierunek=0;kierunek<maksymalny_kierunek[plansza[px][py]];kierunek++)
					for(odleglosc=1;odleglosc<liczba_pol[plansza[px][py]];odleglosc++){
						dx=(odleglosc-1)*WX[plansza[px][py]][kierunek];
						dy=(odleglosc-1)*WY[plansza[px][py]][kierunek];

						if(odleglosc>=2 && plansza[px+dx][py+dy]!=12) break; //sprawdzenie drogi figury
						dx=odleglosc*WX[plansza[px][py]][kierunek];	
						dy=odleglosc*WY[plansza[px][py]][kierunek];
						
						if(px+dx>=0 && px+dx<8 && py+dy>=0 && py+dy<8) //ruch w szachownicy
							if(plansza[px+dx][py+dy]==12 || plansza[px+dx][py+dy]<=5){ //pole końcowe puste/fig wroga
								if(plansza[px][py]!=11 || (plansza[px+dx][py+dy] == 12 && dx==0) || (plansza[px+dx][py+dy]!=12 && dx!=0)){
									ruch_fig=plansza[px][py];
									bita_fig=plansza[px+dx][py+dy];
									plansza[px+dx][py+dy]=plansza[px][py];
									plansza[px][py]=12;
									
									if(plansza[px+dx][py+dy]==11 && py+dy==7) //pion doszedl do konca
										plansza[px+dx][py+dy]=7;	
									wynik=ruch(tryb-1, &px_pom, &py_pom, &k_pom, &o_pom);
									plansza[px][py] = ruch_fig; //cofniecie ruchu
									plansza[px+dx][py+dy]=bita_fig;
									if(wynik>=wmax){wmax=wynik;*x=px,*y=py, *k=kierunek, *o=odleglosc;} //szukanie najlepszego ruchu
								}
							}
					}
	return wmax;
	}
	else{//gracz
		for(px=0,wmin=100*WYGRANA;px<8;px++)
			for(py=0;py<8;py++)
             if(plansza[px][py]>=0 && plansza[px][py]<6) //sprawdzenie czy na polu znajduje sie figura gracza
				for(kierunek=0;kierunek<maksymalny_kierunek[plansza[px][py]];kierunek++)
					for(odleglosc=1;odleglosc<liczba_pol[plansza[px][py]];odleglosc++){
						dx=(odleglosc-1)*WX[plansza[px][py]][kierunek];
						dy=(odleglosc-1)*WY[plansza[px][py]][kierunek];

						if(odleglosc>=2 && plansza[px+dx][py+dy]!=12) break; //jesli po drodze niepuste
						dx=odleglosc*WX[plansza[px][py]][kierunek];	
						dy=odleglosc*WY[plansza[px][py]][kierunek];
						
						if(px+dx>=0 && px+dx<8 && py+dy>=0 && py+dy<8) //sprawdzenie czy ruch miesci sie w szachownicy
							if(plansza[px+dx][py+dy]==12 || plansza[px+dx][py+dy]>=6){ //sprawdzenie zawartosci
								if(plansza[px][py]!=5 || (plansza[px+dx][py+dy] == 12 && dx==0) || (plansza[px+dx][py+dy]!=12 && dx!=0)){
									ruch_fig=plansza[px][py];
									bita_fig=plansza[px+dx][py+dy];
									plansza[px+dx][py+dy]=plansza[px][py];
									plansza[px][py]=12;
									
									if(plansza[px+dx][py+dy]==5 && py+dy==0) //promocja pionka
										plansza[px+dx][py+dy]=1;	
									
									wynik=ruch(tryb-1, &px_pom, &py_pom, &k_pom, &o_pom);
									plansza[px][py] = ruch_fig; //cofniecie ruchu
									plansza[px+dx][py+dy]=bita_fig;
									if(wynik<=wmin){wmin=wynik;*x=px,*y=py, *k=kierunek, *o=odleglosc;}
								}
							}
					}
	return wmin; 
	}
}
