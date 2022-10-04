#include <stdio.h>

#define P_LENA128 "./Lena_128x128_yuv400.raw"
#define P_LENA512_BILINEAR "./Lena_512x512_yuv400_bilinear.raw"
typedef unsigned char PIXEL;
typedef PIXEL img128[128][128];
typedef PIXEL img512[512][512];

void get128(img128 img, char * path);
void put128(img128 img);
void put512(img512 img);
void save512(img512 img, char * path);
void bilinear(img128 input, img512 output);

int main(void){
    img128 LENA;
    get128(LENA, P_LENA128);
    //put128(LENA);

    img512 LENA_BILINEAR;
    bilinear(LENA, LENA_BILINEAR);
    //put512(LENA_BILINEAR);
    save512(LENA_BILINEAR, P_LENA512_BILINEAR);

    return 0;
}

void get128(img128 img, char * path){
    FILE*fp = fopen(path, "r");
    if (fp) {
        for(int r = 0; r < 128; r++){
            for(int c = 0; c < 128; c++)
                img[r][c] = fgetc(fp);
        }
    }
    fclose(fp);
}
void put128(img128 img){
    for(int r = 0; r < 128; r++){
        for(int c = 0; c < 128; c++)
            printf("%d ", img[r][c]);
        printf("\n");
    }
}
void put512(img512 img){
    for(int r = 0; r < 512; r++){
        for(int c = 0; c < 512; c++)
            printf("%d ", img[r][c]);
        printf("\n");
    }
}

void save512(img512 img, char * path){
    FILE*fp = fopen(path, "wb");
    if (fp) {
        for(int r = 0; r < 512; r++){
            for(int c = 0; c < 512; c++)
                 fputc(img[r][c], fp);
        }
    }
    fclose(fp);
}

void bilinear(img128 input, img512 output){
    //입력이미지 픽셀마다 5*4 interpolation
    for(int r = 0; r < 128-1; r++){
        for(int c = 0; c < 128-1; c++){
            PIXEL v1 = input[r][c];
            PIXEL v2 = input[r][c+1];
            PIXEL v3 = input[r+1][c];
            PIXEL v4 = input[r+1][c+1];
            //첫번째 줄(init 1개 + 3개 = 4열)
            output[r*4+2][c*4+2] = v1;
            for(int i = 1; i < 4; i++)
                output[r*4+2][(c*4)+i+2] = (v1*(4-i) + v2*i) >> 2;
            
            //두번째 줄(init 1개 + 3개 = 4열)
            output[(r+1)*4+2][c*4+2] = v3;
            for(int i = 1; i < 4; i++)
                output[(r+1)*4+2][(c*4)+i+2] = (v3*(4-i) + v4*i) >> 2;
            
            //4개의 줄 사이 채우기(4열에 대하여 * 빈칸 3줄 = 12개)
            for(int i = 0; i < 4; i++){
                for(int j = 1; j < 4; j++)
                    output[r*4+j+2][c*4+i+2] = (output[r*4+2][c*4+i+2]*(4-j) + output[(r+1)*4+2][c*4+i+2]*j) >> 2;
            }
        }
    }
    //마지막 열 interpolation
    for(int r = 0; r < 128-1; r++){
        PIXEL v1 = input[r][127];
        PIXEL v3 = input[r+1][127];
        output[r*4+2][510] = v1;
        output[(r+1)*4+2][510] = v3;
        for(int j = 1; j < 4; j++)
            output[r*4+j+2][510] = (output[r*4+2][510]*(4-j) + output[(r+1)*4+2][510]*j) >> 2;
    }
    
	//영역 밖 interpolation, 주변값 복사
	for (int r = 2; r < 512 - 1; r++) {		
        output[r][0] = output[r][2]; // 왼쪽
        output[r][1] = output[r][2]; // 왼쪽
		output[r][511] = output[r][510]; // 오른쪽
	}
	for (int c = 0; c < 512; c++) {
		output[0][c] = output[2][c]; // 위
		output[1][c] = output[2][c]; // 위
		output[511][c] = output[510][c]; // 아래
	}
}