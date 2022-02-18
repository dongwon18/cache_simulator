#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct inst{
    char inst[2];
    char data[9];
    int t0, s0, b0, t1, s1, b1;
} INST;

typedef struct type0{
    int tag;
    int valid;
} TYPE0_CACHE;

typedef struct type1{
    int tag0;
    int tag1;
    int valid0;
    int valid1;
    int dirty0;
    int dirty1;
    int lru;
} TYPE1_CACHE;

int inst_count(FILE *fp);
void inst_decode(INST *inst, FILE *fp, int inst_cnt);
int hextobin(char hex_string[], int hex_to_bin[32]);
void cache0_run(TYPE0_CACHE cache0[], INST inst[], int inst_cnt);
void cache1_run(TYPE1_CACHE cache1[], INST inst[], int inst_cnt);


int main(int argc, char **argv) {
    char type = *argv[1];
    char file_no = *argv[2];
    char filename[100];
    strcpy(filename, "trace");
    strcat(filename, argv[2]);
    //printf("file_no %c\n", file_no);
    //printf("error??");
    strcat(filename, ".txt");
    //printf("%d",argc);
    //printf("%s %s %s", argv[0],argv[1],argv[2]);
    //printf("%c\n", type);
    //printf("%s\n", filename);
    //printf("%d", strcmp(type, "0"));

    if((argc != 3) || ((type != '0') &&(type != '1'))){
        printf("wrong input\n");
        return 1;
    }else{
        FILE *fp = fopen(filename, "r");
        if(fp == NULL){
            printf("file opening failed\n");
        }

        int inst_cnt = inst_count(fp);

        INST *inst = (INST *)malloc(inst_cnt*sizeof(INST));
        TYPE0_CACHE *cache0 = (TYPE0_CACHE *)calloc(1024, sizeof(TYPE0_CACHE));
        TYPE1_CACHE *cache1 = (TYPE1_CACHE *)calloc(512, sizeof(TYPE1_CACHE));
        //printf("dymanic finished\n");

        inst_decode(inst, fp, inst_cnt);
        //printf("inst_decode finished\n");

        if(type == '0'){
            cache0_run(cache0, inst, inst_cnt);
        }else if(type == '1'){
            cache1_run(cache1, inst, inst_cnt);
        }


        free(cache0);
        free(cache1);
        free(inst);
        fclose(fp);
        return 0;
    }

}

void cache0_run(TYPE0_CACHE cache0[], INST inst[], int inst_cnt){
    int set_index;
    int miss = 0;
    int mem_wr = 0;

    for(int i = 0; i < inst_cnt; i++){
        set_index = inst[i].s0;
        //printf("%d cache0 %d valid %d", i, set_index, cache0[set_index].valid);
        if(inst[i].inst[0] == 'L'){
            if(cache0[set_index].valid == 0){
                miss++;
                cache0[set_index].tag = inst[i].t0;
                cache0[set_index].valid = 1;
                //printf("miss-cold");
           }else{
                if(cache0[set_index].tag != inst[i].t0){
                    miss++;
                    cache0[set_index].tag = inst[i].t0;
                    cache0[set_index].valid = 1;
                   // printf("miss-tag not matched");
                }
            }
        }else if(inst[i].inst[0] == 'S'){
            if((cache0[set_index].tag == inst[i].t0) && cache0[set_index].valid == 1){
                //  store hit-write through
                mem_wr++;
                cache0[set_index].valid = 1;
               // printf("hit, mem write");
            }else{
                //  store miss - no write allocate
                miss++;
                mem_wr++;
                //printf("store miss");
            }
        }
        //printf("\n");
    }

    printf("%d %d", miss, mem_wr);
}

void cache1_run(TYPE1_CACHE cache1[], INST inst[], int inst_cnt){
    int set_index;
    int miss = 0;
    int mem_wr = 0;

    for(int i = 0; i < inst_cnt; i++){
        set_index = inst[i].s1;
        //printf("%d cache1 %d valid %d%d ", i, set_index, cache1[set_index].valid0, cache1[set_index].valid1);
        if(inst[i].inst[0] == 'L'){
            if((cache1[set_index].valid0 == 0) && (cache1[set_index].valid1 == 0)){
                //  cold miss
                miss++;
                cache1[set_index].tag0 = inst[i].t1;
                cache1[set_index].valid0 = 1;
                cache1[set_index].lru = 1;
                //printf("both miss-cold");
            }else if((cache1[set_index].valid0 == 1) && (cache1[set_index].valid1 == 0)){
                if(cache1[set_index].tag0 != inst[i].t1){
                    miss++;
                    cache1[set_index].tag1 = inst[i].t1;
                    cache1[set_index].valid1 = 1;
                    cache1[set_index].lru = 0;
                    //printf("tag1 cold");
                }
            }else if((cache1[set_index].valid0 == 0) && (cache1[set_index].valid1 == 1)){
                if(cache1[set_index].tag1 != inst[i].t1){
                    miss++;
                    cache1[set_index].tag0 = inst[i].t1;
                    cache1[set_index].valid0 = 1;
                    cache1[set_index].lru = 1;
                   // printf("tag 0 cold");
                }
            }else{
                // both valid
                if(cache1[set_index].tag0 == inst[i].t1){
                    cache1[set_index].lru = 1;
                    //printf("hit -tag0");
                }else if(cache1[set_index].tag1 == inst[i].t1){
                    cache1[set_index].lru = 0;
                    //printf("hit -tag1");
                }else{
                    miss++;
                    if(cache1[set_index].lru == 0){
                        cache1[set_index].tag0 = inst[i].t1;
                        cache1[set_index].lru = 1;
                       // printf("miss - tag0 is old ");
                        if(cache1[set_index].dirty0 == 1){
                            mem_wr++;
                           // printf("tag0 dirty");
                        }
                        cache1[set_index].dirty0 = 0;
                    }else{
                        cache1[set_index].tag1 = inst[i].t1;
                        cache1[set_index].lru = 0;
                        //printf("miss - tag1 is old ");
                        if(cache1[set_index].dirty1 == 1){
                            mem_wr++;
                            //printf("tag1 dirty");
                        }
                        cache1[set_index].dirty1 = 0;
                    }
                }
            }
        }else if(inst[i].inst[0] == 'S'){
            if((cache1[set_index].tag0 == inst[i].t1) &&(cache1[set_index].valid0 == 1)){
                // tag0 hit
                if(cache1[set_index].dirty0 == 1){
                    //mem_wr++;
                    //printf("tag0 dirty ");
                }
                cache1[set_index].dirty0 = 1;
                cache1[set_index].valid0 = 1;
                cache1[set_index].lru = 1;
                //printf("store- tag0 hit");
            }else if((cache1[set_index].tag1 == inst[i].t1)&& (cache1[set_index].valid1 == 1)){
                // tag1 hit
                if(cache1[set_index].dirty1 == 1){
                    //mem_wr++;
                    //printf("tag1 dirty ");
                }
                cache1[set_index].dirty1 = 1;
                cache1[set_index].valid1 = 1;
                cache1[set_index].lru = 0;
                //printf("store- tag1 hit");
            }else{
                miss++;
                if(cache1[set_index].lru == 0){
                    // cold or tag0 is old
                    if(cache1[set_index].dirty0 == 1){
                        mem_wr++;
                        //printf("tag0 dirty ");
                    }
                    cache1[set_index].tag0 = inst[i].t1;
                    cache1[set_index].dirty0 = 1;
                    cache1[set_index].valid0 = 1;
                    cache1[set_index].lru = 1;
                    //printf("miss-tag0 old or cold");
                }else if(cache1[set_index].lru == 1){
                    // tag1 is old
                    if(cache1[set_index].dirty1 == 1){
                        mem_wr++;
                        //printf("tag1 dirty ");
                    }
                    cache1[set_index].tag1 = inst[i].t1;
                    cache1[set_index].dirty1 = 1;
                    cache1[set_index].valid1 = 1;
                    cache1[set_index].lru = 0;
                    //printf("miss-tag1 old or cold");
                }
            }
        }
        //printf("\n");
    }
    printf("%d %d", miss, mem_wr);
}

int inst_count(FILE *fp){
    int count = 0;
    char c;

    while((c = fgetc(fp)) != EOF){
        if(c == '\n')
            count++;
    }
    fseek(fp, 0, SEEK_SET);  //  set to first
    return count;
}

void inst_decode(INST *inst, FILE *fp, int inst_cnt){
    char low_inst[100];
    char *token;
    int bin_string[32];
    int t0 = 0;
    int s0 = 0;
    int b0 = 0;
    int t1 = 0;
    int s1 = 0;
    int b1 = 0;


    for(int i = 0; i < inst_cnt; i++){
        fgets(low_inst, sizeof(low_inst), fp);

       // printf("fgets \n");
        //printf("%d", sizeof(low_inst));
        //low_inst[11] = '\0';
       // printf("insert null");
        low_inst[strlen(low_inst) - 1] = '\0';
       // printf("%d", sizeof(low_inst));
       // printf("\nlow inst %s\n", low_inst);
        token = strtok(low_inst, " ");
       // printf("token length");
        strcpy((inst[i].inst), token);
        //printf("%s %d", inst[i].inst, sizeof(inst[i].inst));

        token = strtok(NULL, " ");
        strcpy((inst[i].data), token);
       // printf("token \n");
        //printf("\n inst[i].data %s %d", inst[i].data, sizeof(inst[i].data));
        hextobin((inst[i].data), bin_string);
      //  printf("hextobin\n");

        for(int j = 0; j < 18; j++){
            t0 += bin_string[j]*(int)pow(2, 17 - j);
        }
        for(int j = 0; j < 10; j++){
            s0 += bin_string[18 + j]*(int)pow(2, 9 - j);
        }
        for(int j = 0; j < 4; j++){
            b0 += bin_string[28 + j]*(int)pow(2, 3 - j);
        }
        for(int j = 0; j < 17; j++){
            t1 += bin_string[j] * (int)pow(2, 16 - j);
        }
        for(int j = 0; j < 9; j++){
            s1 += bin_string[17 + j] * (int)pow(2, 8 - j);
        }
        for(int j = 0; j < 6; j++){
            b1 += bin_string[26 + j] * (int)pow(2, 5 - j);
        }
       // printf("decode\n");
        inst[i].t0 = t0;
        inst[i].s0 = s0;
        inst[i].b0 = b0;
        inst[i].t1 = t1;
        inst[i].s1 = s1;
        inst[i].b1 = b1;
        //printf("%d %d %d %d %d %d %d\n", i, inst[i].t0, inst[i].s0, inst[i].b0, inst[i].t1, inst[i].s1, inst[i].b1);
        t0 = 0;
        s0 = 0;
        b0 = 0;
        t1 = 0;
        s1 = 0;
        b1 = 0;
    }

}

int hextobin(char hex_string[], int hex_to_bin[32]){
    //int hex_to_bin[32];
    //printf("%s\n\n", hex_string);
        for(int j = 0; j < 8; j++){
            char hexa = hex_string[j];
            // change to lower case alphabet
            if(hexa >= 'A' && hexa <= 'Z'){
                hexa = hexa + 32;
            }
           // printf("hexa %c\n", hexa);


            if(hexa == '0'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == '1'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == '2'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == '3'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == '4'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == '5'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == '6'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == '7'){
                hex_to_bin[4*j] = 0;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == '8'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == '9'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == 'a'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == 'b'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 0;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == 'c'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == 'd'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 0;
                hex_to_bin[4*j + 3] = 1;
            }else if(hexa == 'e'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 0;
            }else if(hexa == 'f'){
                hex_to_bin[4*j] = 1;
                hex_to_bin[4*j + 1] = 1;
                hex_to_bin[4*j + 2] = 1;
                hex_to_bin[4*j + 3] = 1;
            }else{
                return 1;
            }

        }
        /**
        printf("\n");

            for(int k = 0; k < 32; k++){
                printf("%d", hex_to_bin[k]);
                if(k % 4 == 0)
                    printf("_");

            }
            printf("\nhextobin end");*/


    return 0;
}
