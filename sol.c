#include <stdio.h>

#define P_LENA128 "./Lena_128x128_yuv400.raw"
#define P_LENA512_BILINEAR "./Lena_512x512_yuv400_bilinear.raw"
#define P_LENA512_3THLAGRANGE "./Lena_512x512_yuv400_3thlagrange.raw"
#define P_LENA512_SIXTAB "./Lena_512x512_yuv400_sixtab.raw"

typedef unsigned char PIXEL;
typedef PIXEL img128[128][128];
typedef PIXEL img512[512][512];

void get128(img128 img, char * path);
void put128(img128 img);
void put512(img512 img);
void save512(img512 img, char * path);

void bilinear(img128 input, img512 output);
void thirdLagrange(img128 input, img512 output);
void sixtab(img128 input, img512 output);

int main(void){
    img128 LENA;
    get128(LENA, P_LENA128);
    //put128(LENA);

    img512 LENA_BILINEAR;
    bilinear(LENA, LENA_BILINEAR);
    //put512(LENA_BILINEAR);
    save512(LENA_BILINEAR, P_LENA512_BILINEAR);

    img512 LENA_3THLAGRANGE;
    thirdLagrange(LENA, LENA_3THLAGRANGE);
    //put512(LENA_3THLAGRANGE);
    save512(LENA_3THLAGRANGE, P_LENA512_3THLAGRANGE);

    img512 LENA_SIXTAB;
    sixtab(LENA, LENA_SIXTAB);
    //put512(LENA_3THLAGRANGE);
    save512(LENA_SIXTAB, P_LENA512_SIXTAB);

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

void thirdLagrange(img128 input, img512 output){
    for(int r = 0; r < 128; r++){
        for(int c = 0; c < 124; c+=3){
            PIXEL y0 = input[r][c];
            PIXEL y1 = input[r][c+1];
            PIXEL y2 = input[r][c+2];
            PIXEL y3 = input[r][c+3];

            int i = 0;
            if(c == 0) i = -2;
            for (; i < 12; i++) {
				int n0 = (i - 4) * (i - 8) * (i - 12); 
				int n1 = (i - 8) * (i - 12) * (i - 0);
				int n2 = (i - 0) * (i - 4) * (i - 12);
				int n3 = (i - 0) * (i - 4) * (i - 8); 

				int l0 = -n0;
				int l1 = n1 * 3;
				int l2 = -n2 * 3;
				int l3 = n3;

				output[r*4+2][c*4+i+2] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
			}
        }

        //바운더리
        int c = 124;
        PIXEL y0 = input[r][c];
        PIXEL y1 = input[r][c+1];
        PIXEL y2 = input[r][c+2];
        PIXEL y3 = input[r][c+3];
        int i = 0;
        for (; i < 14; i++) {
            int n0 = (i - 4) * (i - 8) * (i - 12);
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[r*4+2][c*4+i+2] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }
    
    //put512(output);
    for(int c = 0; c < 512; c++){
        for(int r = 0; r < 124; r+=3){
            PIXEL y0 = output[r*4+2][c];
            PIXEL y1 = output[(r+1)*4+2][c];
            PIXEL y2 = output[(r+2)*4+2][c];
            PIXEL y3 = output[(r+3)*4+2][c];
            
            int i = 0;
            if(r == 0) i -= 2;
            for (; i < 12; i++) {
				int n0 = (i - 4) * (i - 8) * (i - 12);
				int n1 = (i - 8) * (i - 12) * (i - 0);
				int n2 = (i - 0) * (i - 4) * (i - 12);
				int n3 = (i - 0) * (i - 4) * (i - 8); 

				int l0 = -n0;
				int l1 = n1 * 3;
				int l2 = -n2 * 3;
				int l3 = n3;

				output[r*4+i+2][c] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
			}
        }

        //바운더리
        int r = 124;
        PIXEL y0 = output[r*4+2][c];
        PIXEL y1 = output[(r+1)*4+2][c];
        PIXEL y2 = output[(r+2)*4+2][c];
        PIXEL y3 = output[(r+3)*4+2][c];
        int i = 0;
        for (; i < 14; i++) {
            int n0 = (i - 4) * (i - 8) * (i - 12); 
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[r*4+i+2][c] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }
}


void sixtab(img128 input, img512 output){

    //간격벌려서 저장
    for(int r = 0; r < 128; r++){
        for(int c = 0; c < 128; c++)
            output[r*4+2][c*4+2] = input[r][c]; //510 * 510
    }

    //줄 진행
    for(int r = 2; r < 512; r+=4){
        for(int c = 2; c < 512 - 20; c+=4){
            PIXEL A = output[r][c];
            PIXEL C = output[r][c+4];
            PIXEL G = output[r][c+8];
            PIXEL M = output[r][c+12];
            PIXEL R = output[r][c+16];
            PIXEL T = output[r][c+20];
            int h = ((A-5*C+20*G+20*M-5*R+T)+16)>>5;
            output[r][c+10] = h;
        }
    }

    //left padding
    for(int r = 2; r < 512; r+=4){
        PIXEL y0 = output[r][2];
        PIXEL y1 = output[r][6];
        PIXEL y2 = output[r][10];
        PIXEL y3 = output[r][14];

        for (int i = 2; i <= 6; i+=4) {
            int n0 = (i - 4) * (i - 8) * (i - 12); 
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[r][2+i] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }

    //right padding
    for(int r = 2; r < 512; r+=4){
        PIXEL y0 = output[r][498];
        PIXEL y1 = output[r][502];
        PIXEL y2 = output[r][506];
        PIXEL y3 = output[r][510];

        for (int i = 6; i <= 10; i+=4) {
            int n0 = (i - 4) * (i - 8) * (i - 12); 
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[r][498+i] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }
    
    //행 진행
    for(int c = 2; c < 512; c+=2){
        for(int r = 2; r < 512 - 20; r+=4){
            PIXEL A = output[r][c];
            PIXEL C = output[r+4][c];
            PIXEL G = output[r+8][c];
            PIXEL M = output[r+12][c];
            PIXEL R = output[r+16][c];
            PIXEL T = output[r+20][c];
            int h = ((A-5*C+20*G+20*M-5*R+T)+16)>>5;
            output[r+10][c] = h;
        }
    }


    //top padding
    for(int c = 2; c < 512; c+=2){
        PIXEL y0 = output[2][c];
        PIXEL y1 = output[6][c];
        PIXEL y2 = output[10][c];
        PIXEL y3 = output[14][c];

        for (int i = 2; i <= 6; i+=4) {
            int n0 = (i - 4) * (i - 8) * (i - 12); 
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[2+i][c] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }

    //bottom padding
    for(int c = 2; c < 512; c+=2){
        PIXEL y0 = output[498][c];
        PIXEL y1 = output[502][c];
        PIXEL y2 = output[506][c];
        PIXEL y3 = output[510][c];

        for (int i = 6; i <= 10; i+=4) {
            int n0 = (i - 4) * (i - 8) * (i - 12); 
            int n1 = (i - 8) * (i - 12) * (i - 0);
            int n2 = (i - 0) * (i - 4) * (i - 12);
            int n3 = (i - 0) * (i - 4) * (i - 8); 

            int l0 = -n0;
            int l1 = n1 * 3;
            int l2 = -n2 * 3;
            int l3 = n3;

            output[498+i][c] = (PIXEL)((((y0 * l0) + (y1 * l1) + (y2 * l2) + (y3 * l3) + 192) >> 7) / 3);
        }
    }

    
    for(int r = 2; r < 512; r+=4){
        for(int c = 2; c < 512; c+=4){
            output[r][c+1] = (output[r][c]+output[r][c+2])/2;
            output[r][c+3] = (output[r][c+2]+output[r][c+4])/2;
            output[r+2][c+1] = (output[r+2][c]+output[r+2][c+2])/2;
            output[r+2][c+3] = (output[r+2][c+2]+output[r+2][c+4])/2;

            output[r+1][c] = (output[r][c]+output[r+2][c])/2;
            output[r+3][c] = (output[r+2][c]+output[r+4][c])/2;
            output[r+1][c+2] = (output[r][c+2]+output[r+2][c+2])/2;
            output[r+3][c+2] = (output[r+2][c+2]+output[r+4][c+2])/2;

            output[r+1][c+1] = (output[r+2][c]+output[r][c+2])/2;
            output[r+1][c+3] = (output[r][c+2]+output[r+2][c+4])/2;
            output[r+3][c+1] = (output[r+2][c]+output[r+4][c+2])/2;
            output[r+3][c+3] = (output[r+2][c+4]+output[r+4][c+2])/2;
        }
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
