#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define M 347 // kelime sayisindan buyuk en kucuk asal sayi
#define MM 346 // ikinci hashleme icin kelime sayisinden kucuk ve yakin bir asal sayi

struct data{
	char key[20]; // hash tablosundaki kelimeler key-value seklinde tutuldu
	char value[20];
};

struct hashtable_item{
	struct data *item;
	/*
	flag = 1 data var
	flag = 0 data yok
	*/
	int flag; 
};

struct hashtable_item *dictionary = NULL; // sozluk hash tablosu

struct hashtable_item *error_dictionary = NULL; // error hash tablosu

int advice_numbers = 0; // kullaniciya onerilen kelimelerin sayisinin tutuldugu degisken
int size = 0; // error tablosunun boyutu ilklendi

int hashcode1(int key){ // ilk hash islemi icin deger donduren fonksiyon
	return (key % M);
}
 
int hashcode2(int key){ // ikinci hash islemi icin deger donduren fonksiyon
	return (key % MM) + 1; // ilk hash deger ile toplanacak
}

/*
kelimelerin sayisal degerleri icin kullanilan fonksiyon
horner methodu kullanilarak kelime icin bir hash degeri dondurulur
kelimenin yerlesecegi indis icin gerekli
*/
int horner_method(char word[20]){
	int hash = 0, i;
	for(i = 0; word[i]; i++){ // kelimelerde buyuk kucuk farkini engellemek icin
	  word[i] = tolower(word[i]); // her bir karakter kucuk haline donusturuldu
	}
	for (i = 0; i < strlen(word); i++)
		hash = (31 * hash) + word[i];
	return abs(hash); // overflow olma durumu icin mutlak degeri aliniyor
}

/* 
hash tablosunun kullanilacagi dizi baslangic degerini aliyor
hashtable text dosyasindan arraye tum degerler gecirilir
*/
void init_array(FILE *file){
	char str1[50]; // her kelime icin kullanilacak
	while (fscanf(file,"%s ",str1) != EOF){
		 // dosyada satir olarak okuma, her bir deger ayri ayri alinir
		int key = horner_method(str1); // kelimenin sayi degeri bulunuyor, key olarak kullanilacak
		int hash1 = hashcode1(key); // ilk hash degeri
		int hash2 = hashcode2(key); // cakisma durumunda kullanilacak ikinci hash degeri
	 
		int index = hash1; 
		// flag = 1 tabloda indis dolu, ikinci hashleme kullanilarak yer aranacak
		if (dictionary[index].flag == 1) { // sozlukte cakismayi engellemek icin ikinci hash degeri burada kullanilir
			int i = 1;
			int control = 0;
			while(control == 0){ // yer bulunana kadar formul kullanilarak devam edilir
				index = (index + (i * hash2)) % M; // yeni indis
				if (dictionary[index].flag == 0){
					control = 1; // bos adres bulundugunda yerlesme icin donguden cikilir
				}
				i++; // indis arttirilir
			}
		}
		dictionary[index].item = (struct data*) malloc(sizeof(struct data)); // kelime, bulunan yere yerlestirilir
		strcpy(dictionary[index].item->key, str1);
		dictionary[index].flag = 1; // flag ayarlaniyor
	}
	fclose(file);
}

/*
ekleme islemi icin kullanilacak fonksiyon
sadece hatali kelime tablosuna kelime eklemek icin kullanilacak
"main" fonksiyonunda kelimeler icin girilen dongude her bir kelime icin hesaplanan
index ve hash2 degerleri parametre olarak gonderilip fonksiyon icerisinde tabloda uygun yer aranir
kelimeler tabloya yerlestirilir
*/

void insert(char *word, char *value, int index, int hash2){
 
	/* eklenecek data icin memory allocation yapiliyor */
	struct data *new_item = (struct data*) malloc(sizeof(struct data));
	strcpy(new_item->key, word);
	strcpy(new_item->value, value);
	
	/* 
	eklenecek kelime tabloda var ise
	cakisma durumu soz konusudur, dizi icerisinde bu dogrultuda bos yer aranir
	*/
	if (error_dictionary[index].flag == 1) {
		int i = 1;
		int control = 0;
		while(control == 0){ // yer bulunana kadar formul kullanilarak devam edilir
			index = (index + (i * hash2)) % M; // yeni indis
			if (error_dictionary[index].flag == 0){
				control = 1; // bos adres bulundugunda yerlesme icin donguden cikilir
			}
			i++; // indis arttirilir
		}
	}
 
	error_dictionary[index].item = new_item; // kelime, ilgili indise yerlestirilir
	error_dictionary[index].flag = 1;
	size++; // tablonun islemler boyunca eklenen boyutu arttirilir
}

/*
kelime arama isleminde kullanilacak fonksiyon, sozluk icerisinde kelime aranir
her bir kelime icin belirlenmis hash degeri parametre olarak gonderildi
cakisma durumunda ekleme islemi icin kullanilan hashleme islemi burada da uygulanir
*/
bool search_in_dict(char word[20], int hash1, int index, int hash2){
	int hasKey = 0;
	
	if(dictionary[index].flag != 1){ // flag 0, herhangi bir kelime bulunamadi
		return false; // "main" icerisinde kullanilmak uzere false dondurulur
	}
	int i = 1;
	while(hasKey == 0){ // kelime tabloda var
		if (dictionary[index].flag == 1 && strcmp(dictionary[index].item->key,word) == 0){ // flag 0 olmadigi surece devam edilir
			hasKey = 1;
			return true;
		}
		index = (index + (i * hash2)) % M; // index ikinci hashleme degeri ile degistiriliyor
		i++;
		if(index == (hash1 / 2) + 1){ // tablo tamamen dolu olduðunda tekrar eden donguleri onlemek icin
			break; // yarý dongu tamamlandiginda (tum kelimeler en az bir kez arandiginda) cikilacak
		}
	}
	return false; // kelime bulunamadi
}
/*
kelime arama islemi icin kullanilacak fonksiyon, hatali sozluk icerisinde kelime aranir
her kelime icin yine belirlenmis hash degerleri parametre olarak gonderilmistir
var ise bulunan kelimeyi, yok ise null dondurecek
*/
char * search_in_err(char word[20], int hash1, int index, int hash2){
	
	if(error_dictionary[index].flag != 1){ // flag 0, herhangi bir kelime bulunamadi
		return NULL;
	}
	int i = 1;
	while(error_dictionary[index].flag == 1){ // kelime tabloda var
		if (strcmp(error_dictionary[index].item->key,word) == 0){ // flag 0 olmadigi surece devam edilir
			return error_dictionary[index].item->value;
		}
		index = (index + (i * hash2)) % M; // index ikinci hashleme degeri ile degistiriliyor
		i++;
		if(index == (hash1 / 2) + 1){ // tablo tamamen dolu olduðunda tekrar eden donguleri onlemek icin
			break; // yarý dongu tamamlandiginda (tum kelimeler en az bir kez arandiginda) cikilacak
		}
	}
	return NULL; // kelime bulunamadi
}

/*
iki kelime arasindaki uzakligi bulan fonksiyon, Levenshtein-Distance
iki kelimenin uzunlugu kullanilarak bir matris olusturulur
her iki kelime icin harf harf matrise yerlestirme yapilir (satir-sutun seklinde)
*/
static int distance (const char * word1,
                     int len1,
                     const char * word2,
                     int len2)
{
    int matrix[len1 + 1][len2 + 1];
    int i;
    for (i = 0; i <= len1; i++) { // word1 harfleriyle satir baslarina yerlestiriliyor
        matrix[i][0] = i;
    }
    for (i = 0; i <= len2; i++) { // word2 harfleriyle sutun baslarina yerlestiriliyor
        matrix[0][i] = i;
    }
    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = word1[i-1]; // ilk kelimenin harflerini tutan degisken, ilk harften baslanir
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = word2[j-1]; // ikinci kelimenin harflerini tutan degisken, ilk harften baslanir
            if (c1 == c2) {
            	// kelimeler icindeki ayný indisteki harf ayný ise bulunan indise bir onceki sayi aynen yerlestirilir
                matrix[i][j] = matrix[i-1][j-1];
            }
            else {
                int delete; // ilk kelimeden ikinci kelimeye kiyasla silinecek harf sayisi (beautifully-beautiful = beautifullyden cikarma islemi)
                int insert; // ilk kelimeye ikinci kelimeye kýyasla eklenecek harf sayisi (book-books = book kelimesine ekleme yapilir)
                int substitute; // ilk kelimede kaydirma isleminin sayisi (relevant-elephant = ele kisimlari ayni, kaydirma yapilir)
                int minimum; // iki kelimenin uzunluk farki olarak ilklenebilir

                delete = matrix[i-1][j] + 1; // silme islemi icin adim sayisi 1 arttirildi (ilgili indiste)
                insert = matrix[i][j-1] + 1; // ekleme islemi ilk kelime icin yapiliyor, adim sayisi 1 artti
                substitute = matrix[i-1][j-1] + 1; // kaydirma matrisin ortak olan kisminda yapilacagindan ikisi de kullanilarak arttirilir
                minimum = delete; // minimum degisikligi (uzakligi) bulmak icin "delete" operasyonu ile ilklendi
                if (insert < minimum) { // daha kucuk bir deger bulunmasi durumunda
                    minimum = insert; // deger degistirilecek
                }
                if (substitute < minimum) {
                    minimum = substitute;
                }
                matrix[i][j] = minimum; // son olarak ilgili harfler icin min uzaklik bulunduklari hucreye yerlestirilir
            }
        }
    }
    return matrix[len1][len2]; // sag altta kalan matris hucresi aradigimiz uzakliktir
}

/*
kullanicinin girmis oldugu kelimenin hem sozlukte hem hatali sozlukte bulunmamasi durumunda 
kullaniciya Levenshtein-Distancei 1 veya 2 olan kelimeler oneri olarak sunulur
oncelikle uzaklik durumu 1 olan kelimeler sunulur, yok ise 2 olanlar sunulur
2den buyukler sunulmaz, kelime aynen kabul edilecektir.
*/

void create_advice(char advice[20][20], char token[20]){
	int advice2_index[50]; // uzakligi 2 olan kelimeler icin dizi tanimi
	int i, j = 0, d, k = 0, l = 0;
	advice_numbers = 0; // girilen bir cumlede her kelime icin fonksiyon kullanilacagindan 0lanir
	bool hasOne = false, hasTwo = false; // 1 ve 2 uzaklikta kelime olup olmadigini kontrol eden degiskenler
	for(i = 0; i<M; i++){ // tum kelimeler icin uzaklik aranir
		if(dictionary[i].flag != 1)
			continue; // sozlukte i. indis bosta, bir islem yapilmadan diger indise gecilir
		/*
		Levenshtein-Distancei bulan fonksiyona, sozlukteki ilgili indisteki kelime
		bu kelimenin uzunlugu, kullanicinin girdigi kelime ve bu kelimenin uzunluklari
		sirasi ile gonderilir,  d uzakligi tutan degiskendir
		*/
		d = distance(dictionary[i].item->key, strlen(dictionary[i].item->key), token, strlen(token));
		if(d == 1){ // uzaklik 1 ise
			hasOne = true; // 1 uzakliga sahip olunup olunmadigini kontrol eden degisken, true olarak setlendi
			strcpy(advice[k], dictionary[i].item->key); // kelime oneri dizisine atilir
			k++;
		}
		if(d == 2){ // 1 uzakligina sahip kelimelerin olmama ihtimali bulundugundan
			hasTwo = true;
			/*
			2 uzakligina sahip kelimenin bulundugu indis uzakligi iki olan kelimeler icin tanimlanan diziye atilir
			*/
			advice2_index[j] = i; 
			j++;
			l++;
		}
	}
	// 1 uzakligina sahip kelime yok ve 2 uzakligina sahip kelime var ise
	if(!hasOne && hasTwo){
		for(i = 0; i<l; i++){ // 2. kelimelerin sayisini tutan "l" degiskeni kadar donguye girilir
			strcpy(advice[k], dictionary[advice2_index[i]].item->key); // oneri dizisine bulunan 2 uzakligindaki kelimeler atilir
			k++;
		}
	}
	advice_numbers = k; // oneri sayisini tutan degisken k'ya esitleniyor, her iki durumda da sayisi k'da saklandi
}

int main ()
{
	/*
	choice = kullanicinin kendisine onerilen kelimeler arasindan hangisini sectigini tutan degisken
	token_index = yeni turetilecek kelimeler bir dizide tutulacak, dizinin indisi icin kullanilacak degisken
	*/
    int choice, token_index = 0;
    char advice[20][20]; // create_advice fonksiyonunda parametre olarak alinan dizi, tavsiye kelimeler burada tutulur
    /*
	c = kullanicin kelime girmeye devam etmesi icin, 1e basilmali
	text = kullanicinin girecegi cumleyi alan karakter dizisi
	*/
    int c, i;
    char text[100];
    FILE *fptrHash = fopen("smallDictionary.txt","r");
    if(fptrHash == NULL){
    	printf(" File can not be opened..");
    	exit(3);
	}
    // kullanilacak olan sozluk ve hatali sozluk icin alan aciliyor
    dictionary = (struct hashtable_item*) malloc(M * sizeof(struct hashtable_item));
    error_dictionary = (struct hashtable_item*) malloc(M * sizeof(struct hashtable_item));
	init_array(fptrHash); // sozluk tablosunu dolduran fonksiyon cagiriliyor, tablo ilklendi
	
	do {
		printf("------------------------------------\n");
		printf(" Enter your text: ");
		scanf(" %[^\n]s", text); // kullanici girdisi aliniyor, "enter" ile sonlanmasi icin regex kullanildi
 		char * token = strtok(text, " "); // cumleyi kelimelerine ayristirmak icin strtok kullanildi
 		char new_tokens[50][50]; // yeni cumlenin yerlestirilecegi kelime dizisi
 		while( token != NULL ) { // girilen kelime sayisi kadar devam edecek
 			int key = horner_method(token); // kelimeler icin index degeri elde ediliyor
			int hash1 = hashcode1(key);
			int hash2 = hashcode2(key);
			
			int index  = hash1;
			// kelime sozluk tablosunda var ise hicbir degisiklik yapilmadan yeni kelime dizisine eklenecek
			bool is_in_dict = search_in_dict(token, hash1, index, hash2);
			if(is_in_dict == false){ // kelime sozlukte yok
				// hatali kelime tablosunda aranir, yok ise null donecek
				char * error_result = search_in_err(token, hash1, index, hash2);
				int j; // hatali kelime tablosunda da yok ise
				if(error_result == NULL){
					create_advice(advice, token); // oneri kelimeler sunulacak
					if(advice_numbers == 0){ // oneri kelime bulunamadiysa
						strcpy(new_tokens[token_index], token); // kelime ayni sekilde yerlestirilir
						token_index++;
						token = strtok(NULL, " "); // bir sonraki kelime gecilir
						continue;
					}
					printf(" %s is not in the dictionary. Did you mean : ", token);
					for(j = 0; j<advice_numbers; j++){ // oneri kelimeler var ise kullaniciya soruluyor
						printf(" (%d) %s  ", j+1, advice[j]);
					}
					printf("\n");
					do{ // kullanicinin tercih ettigi kelimenin rakami alinir
						printf(" Please enter the number of your choice: ");
						scanf("%d", &choice); // sayi kontrol edilir
					}while(choice > advice_numbers || choice < 1);
					/*
					kelime kullanicinin belirlemis oldugu dogru kelimeyle birlikte
					hatali sozluk tablosuna eklenir
					*/
					insert(token, advice[choice - 1], index, hash2);
					strcpy(new_tokens[token_index], advice[choice - 1]); // yeni kelimelerin tutuldugu diziye atiliyor
					token_index++;
				}
				else{ // error tablosunda var, kullanicinin daha once tercih ettigi kelime otomatik duzeltilir
					printf(" You probably mean %s for %s \n", error_result, token);
					strcpy(new_tokens[token_index], error_result);
					token_index++;
				}
			}
			else{
				strcpy(new_tokens[token_index], token); // sozlukte olmasi halinde aynen yeni kelime dizisine atiliyor
				token_index++;
			}
			token = strtok(NULL, " "); // bir sonraki kelimeye geciliyor
		}
		printf("---------------#Your Fixed Sentence#-----------------\n");
		for(i = 0; i<token_index; i++){
			printf(" %s", new_tokens[i]); // duzenlenen cumle kullaniciya gosterilir
		}
		// bir onceki duzenlemede kullanilan degiskenler sifirlaniyor
		token_index = 0;
		memset(new_tokens, 0, sizeof(new_tokens)); 
		memset(text, 0, 50);
		printf("\n Do you want to continue-:(press 1 for yes)\t");
		scanf(" %d", &c); // devam etmesi icin 1 girilmeli
 
	}while(c == 1);
	
    return 0;
}
