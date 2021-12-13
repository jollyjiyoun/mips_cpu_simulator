#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>

int opcode (unsigned int cur_buf, char * assembly_string, int registers_p[], unsigned char data_memory[], int cache1_tag[], int cache1_valid[], int * cache_hit, int * cache_miss, int * cache_hit_miss_total, long *cache_type_p);
int rType (unsigned int cur_buf, char * assembly_string, int registers_p[], unsigned char data_memory[]);
int iType (unsigned int cur_buf, char * assembly_string, int registers_p[],unsigned char data_memory[], int cache1_tag[], int cache1_valid[], int * cache_hit, int *cache_miss, int *cache_hit_miss_total, long *cache_type_p);


int main(int argc, char** argv){
	FILE *fp;
	unsigned int * buffer = NULL;
	unsigned char * buffer_c = NULL;
	long len = 0;
	size_t read;
	char * assembly_string = malloc (sizeof(char) * 30);
	char * print_instruction = malloc(sizeof(char) * 60);
	long n_times = strtol(argv[2], NULL, 10);
	long cache_type = strtol(argv[1], NULL, 10);
	int registers[33];
	unsigned int inst_memory[0x100] = {0};
	unsigned char data_memory[0x10000] = {0};
	int cache1_tag[32];
	int cache1_valid[32];
	int total_instruction_count = 0;
	int unknown_tf = 0;
	long* cache_type_p = &cache_type;

	for(int i = 0; i<33; i++) {
		registers[i] = 0;
	}
	for(int i = 0; i<0x10000; i++){
		data_memory[i] = 0xff;
	}
	for (int i = 0; i< 0x100; i++) {
		inst_memory[i] = 0xffffffff;
	}

	for (int i = 0; i <32; i++) {
		cache1_tag[i] = 0;
		cache1_valid[i] = 0;
	}

	// cache1, cache2 hit / miss initialize
	
	int c1_hit, c1_miss, c_hit_miss_total = 0;
	int *cache_hit = &c1_hit;
	int *cache_miss = &c1_miss;
	int *cache_hit_miss_total = &c_hit_miss_total;


	// if data file is given in input
	if (argc == 5) {

		// open and read data file
		fp = fopen(argv[4], "rb");
		if (fp == NULL) 
			exit(EXIT_FAILURE);
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		rewind(fp);

		buffer_c = (unsigned char*)malloc(sizeof(unsigned char) * len);

		if (buffer_c == NULL) {
                	exit(EXIT_FAILURE);
        	}
		read = fread(buffer_c, 1, len, fp);
		
		for (size_t read_count = 0; read_count < read; read_count++){
			data_memory[read_count] = buffer_c[read_count];
		}
		
		// close File fp
        	fclose(fp);

        	// free heap memory
        	free(buffer_c);

	}

	fp = fopen(argv[3], "rb");
	if(fp == NULL)
		exit(EXIT_FAILURE);
	
	// seek end of file, count length of file, and rewind file to start
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	// allocate buffer memory based on file length retrieved above
	buffer = (unsigned int*)malloc(sizeof(unsigned int) * len);

	if (buffer == NULL) {
		exit(EXIT_FAILURE);
	}

	// read File fp data 4 bytes(32 bits) each until file length(len), and append to buffer
	read = fread(buffer, 4, len, fp);

	// display data in buffer
	for (size_t j = 0; j < read; j++) {

		buffer[j] = __bswap_32(buffer[j]);
		inst_memory[j] = buffer[j]; 

	} 

	unsigned int current_instruction = inst_memory[registers[32]/4];
	unsigned int j = 0;

	if (n_times > 0) {
		total_instruction_count += 1;
	}

	// access instruction one by one and execute
	for (long i = 0; i< n_times; i++) {
		unknown_tf = 0;
                unknown_tf = opcode(current_instruction, assembly_string, registers, data_memory, cache1_tag, cache1_valid, cache_hit, cache_miss, cache_hit_miss_total, cache_type_p);
	
		current_instruction = inst_memory[registers[32]/4];
		
		j += 1;

		if (unknown_tf) {
			registers[32] += 4;
			break;
		}

		total_instruction_count += 1;

		if (i != (n_times-1) && inst_memory[(registers[32]/4)] == ((unsigned int) 0xffffffff)) {
			//printf("unknown instruction\n");
               		registers[32] += 4;
			break;
		}
	}

	/*
	for(int i = 0; i<33; i++) {
		if ( i < 32) {
			printf("$%d: %08x\n", i, registers[i]);
		} else {
			printf("PC: %08x\n", registers[i]);
		}
		
	} */

	printf("Instructions: %d\n", total_instruction_count);
	printf("Total: %d\n", *cache_hit_miss_total);
	if (cache_type == 1) {
		printf("Hits: %d\n", *cache_hit);
		printf("Misses: %d\n", *cache_miss);
	} else if(cache_type == 2) {
		printf("Hits: %d\n", *cache_hit);
                printf("Misses: %d\n", *cache_miss);
	}

	// close File fp
	fclose(fp);

	// free heap memory
	free(buffer);

	free(print_instruction);
		
	free(assembly_string);

	exit(EXIT_SUCCESS);
}

int opcode (unsigned int cur_buf, char * assembly_string, int registers[], unsigned char data_memory[], int cache1_tag[], int cache1_valid[], int * cache_hit, int * cache_miss, int * cache_hit_miss_total, long *cache_type_p) {
	
	unsigned int opcode_val;
	int jtype_target;
	int t1;
	char * num1;
	int unknown = 0;
	
	/* get opcode value by shift left 26 bits -> mask right 6 bits */
	opcode_val = (cur_buf >> 26) & 127;
	
	//printf("%02x\n", opcode_val);	
	switch (opcode_val) {
		case 0:
			//printf("in opcode case 0\n");
			unknown = rType(cur_buf, assembly_string, registers, data_memory);
			break;

		case 2:
			jtype_target = cur_buf & 0x3ffffff;
			t1 = asprintf(&num1, "%d", jtype_target);
			if (t1 == -1) {
                                perror("asprintf");
                                break;
                        } else {
				//printf("jj\n");
				strcat(strcpy(assembly_string, "j "), num1);
                        }
			registers[32] = ((registers[32] + 4) & 0xf0000000) | ((unsigned int)(0x03ffffff & jtype_target)<<2);
                        free(num1);
			break;

		case 3:
			jtype_target = cur_buf & 0x3ffffff;
                        t1 = asprintf(&num1, "%d", jtype_target);
                        if (t1 == -1) {
                                perror("asprintf");
                                break;
                        } else {
				strcat(strcpy(assembly_string, "jal "), num1);
                        }
			registers[31] = registers[32] + 8;
			registers[32] = ((registers[32] + 4) & 0xf0000000) | ((unsigned int)(0x03ffffff & jtype_target)<<2);
                        free(num1);
                        break;

		default:
			unknown = iType(cur_buf, assembly_string, registers, data_memory, cache1_tag, cache1_valid, cache_hit, cache_miss, cache_hit_miss_total, cache_type_p);
			break;
	}

	return unknown;
}

int rType (unsigned int cur_buf, char * assembly_string, int registers[], unsigned char data_memory[]) {
	unsigned int funct_val;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	unsigned int shamt;
	unsigned int rd_shamt; /* 10 */
	unsigned int rt_rd_shamt; /* 15 */
	unsigned int rs_rt; /* 10 */
	unsigned int rs_rt_rd_shamt; /* 20 */
	char * num1;
	char * num2;
	char * num3;
	int t1, t2, t3;	
	char * unknown = "unknown instruction";
	unsigned char data_char;
	char input_val[50];
	int val = 0;
	int unknown_tf = 0;

	funct_val = cur_buf & 63;

	rs = (cur_buf >> 21) & 31;
	rt = (cur_buf >> 16) & 31;
	//rt = (cur_buf >> 16) & 31;
	rd = (cur_buf >> 11) & 31;
	shamt = (cur_buf >> 6) & 31;
	rd_shamt = (cur_buf >> 6) & 0x3ff;
	rt_rd_shamt = (cur_buf >> 6) & 0x7fff;
	rs_rt = (cur_buf >> 16) & 0x3ff;
	rs_rt_rd_shamt = (cur_buf >> 6) & 0xfffff;

	switch (funct_val) {
		case 0x20: //add
			if (shamt) {
				strcpy(assembly_string, unknown);
				break;
			}
			t1 = asprintf(&num1, "%u", rd);
			t2 = asprintf(&num2, "%u", rs);
			t3 = asprintf(&num3, "%u", rt);
			if (t1 == -1 || t2 == -1 || t3 == -1) {
				perror("asprintf");
				break;
			} else {
				strcat(strcpy(assembly_string, "add $"), num1);
				strcat(strcat(assembly_string, ", $"), num2);
				strcat(strcat(assembly_string, ", $"), num3);	
			}
			registers[rd] = registers[rs] + registers[rt];
			registers[32] +=4;

			free(num1);
			free(num2);
			free(num3);	
			break;
		case 0x21: //addu
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			//printf("in rType 0x21\n");
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "addu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			registers[rd] = registers[rs] + registers[rt];
			registers[32] +=4;

			free(num1);
			free(num2);
			free(num3);
                        break;
		case 0x24: //and
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			//printf("in rType 0x24\n");
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "and $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			registers[rd] = registers[rs] & registers[rt];
                        registers[32] += 4;
			free(num1);
			free(num2);
			free(num3);
                        break;
		case 0x1a: // div
			if (rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			//printf("in rType 0x1a\n");
                        t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
				break;
                        } else {
                               	strcat(strcpy(assembly_string, "div $"), num1);
                               	strcat(strcat(assembly_string, ", $"), num2);
                       	}
			free(num1);
                        free(num2);
			unknown_tf = 1;
                       	break;

		case 0x1b: //divu
			if (rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "divu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                        }
			free(num1);
                        free(num2);
			unknown_tf = 1;
                        break;

		case 0x09: //jalr
			if (rt || shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "jalr $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                        }
			free(num1);
                        free(num2);
			unknown_tf = 1;
                        break;

		case 0x08: //jr
			if (rt_rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        if (t1 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "jr $"), num1);
                        }

			registers[32] = registers[rs];
			free(num1);
                        break;

		case 0x10: //mfhi
			if (rs_rt || shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        if (t1 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "mfhi $"), num1);
                        }
			free(num1);
			unknown_tf = 1;
                        break;

		case 0x12: //mflo
			if (rs_rt || shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        if (t1 == -1) {
                                perror("asprintf");
				break;
                        } else {
                                strcat(strcpy(assembly_string, "mflo $"), num1);
                        }
			free(num1);
			unknown_tf = 1;
                        break;

		case 0x11: //mthi
			if (rt_rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        if (t1 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "mthi $"), num1);
                        }
			free(num1);
			unknown_tf = 1;
                        break;

		case 0x13: //mtlo
			if (rt_rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        if (t1 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "mtlo $"), num1);
                        }
			free(num1);
			unknown_tf = 1;
                        break;

		case 0x18: //mult
			if (rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "mult $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                        }
			free(num1);
                        free(num2);
			unknown_tf = 1;
                        break;

		case 0x19: //multu
			if (rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "multu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                        }
			free(num1);
                        free(num2);
			unknown_tf = 1;
                        break;

		case 0x27: //nor
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
			t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "nor $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
				strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x25: //or
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "or $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			registers[rd] = registers[rs] | registers[rt];
                        registers[32] += 4;
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x00: //sll
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", shamt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sll $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			registers[rd] = registers[rt] << shamt;
			registers[32] += 4;
			//printf("mod!! registers[%d] : %u\n", rd, registers[rd]);
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x04: //sllv
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sllv $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x2a: //slt
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "slt $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			if (registers[rs] < registers[rt]) {
				registers[rd] = 1;
			} else {
				registers[rd] = 0;
			}
			registers[32] += 4;
			//printf("mod!! registers[%d] : %u\n", rd, registers[rd]);
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x2b: //sltu
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sltu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			if (registers[rs] < (unsigned int)registers[rt]) {
                                registers[rd] = 1;
                        } else {
                                registers[rd] = 0;
                        }
                        registers[32] += 4;
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x03: //sra
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", shamt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sra $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x07: //srav
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "srav $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x02: //srl
			t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", shamt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "srl $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			registers[rd] = (int)((unsigned int)registers[rt] >> shamt);
			registers[32] += 4;
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x06: //srlv
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "srlv $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x22: //sub
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sub $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			registers[rd] = registers[rs] - registers[rt];
			registers[32] += 4;
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x23: //subu
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "subu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			registers[rd] = registers[rs] - registers[rt];
                        registers[32] += 4;
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0c: //syscall
			
			if (rs_rt_rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			if (registers[2] == 1) {
				printf("%d", registers[4]);
			} else if (registers[2] == 4) {
				//printf("%08x\n", registers[4]);
				data_char = data_memory[registers[4] - 0x10000000];
				val = 0;
				while (data_char != 0) {
					printf("%c", data_char); 
					data_char = data_memory[registers[4]+ val+1 - 0x10000000];
					val += 1;
                                	//printf("%08x\n", (int)data_memory[registers[2]+signed_imm + i-0x10000000]);
                                	//printf("lw val: %08x\n", lw_val);

                        	}
			} else if (registers[2] == 5) {
				scanf("%s", input_val);
				registers[2] = strtol(input_val, NULL, 10);

			}

			registers[32] += 4;
			strcpy(assembly_string, "syscall");

                        break;

		case 0x26: //xor
			if (shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
                        t1 = asprintf(&num1, "%u", rd);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%u", rt);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "xor $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", $"), num3);
                        }
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		default:
			//printf("in rType default\n");
			strcpy(assembly_string, unknown);
                        break;
	}

	return unknown_tf;
}


int iType (unsigned int cur_buf, char * assembly_string, int registers[], unsigned char data_memory[], int cache1_tag[], int cache1_valid[], int * cache_hit, int * cache_miss, int * cache_hit_miss_total, long *cache_type_p) {

	int16_t signed_imm;
	//unsigned int unsigned_imm;
	//unsigned int off;
        unsigned int op; /* 6 */
        unsigned int rs; /* 5 */
        unsigned int rt; /* 5 */
        char * num1;
        char * num2;
        char * num3;
        int t1, t2, t3;
        char * unknown = "unknown instruction";
	int lw_val = 0;
	char temp;
	int load_address = 0;
	int cache_tag = 0;
	int cache_index = 0;
	int unknown_tf = 0;

	//unsigned_imm = cur_buf & 0xffff;
	//off = cur_buf & 0xffff;

	signed_imm = cur_buf & 0xffff;

        rs = (cur_buf >> 21) & 31;
        rt = (cur_buf >> 16) & 31;

	op = (cur_buf >> 26) & 0x2ffffff;

        switch (op) {
                case 0x08: //addi
                        t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "addi $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }

			registers[rt] = registers[rs] + signed_imm;
			registers[32] += 4;

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

                case 0x09: //addiu
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "addiu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			registers[rt] = registers[rs] + signed_imm;
                        registers[32] += 4;
                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0c: //andi
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "andi $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			
			registers[rt] = registers[rs] & ((unsigned int) 0x0000ffff & signed_imm);
			registers[32] += 4;
                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0d: //ori
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "ori $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			
			registers[rt] = registers[rs] | signed_imm;
			registers[32] += 4;

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0e: //xori
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "xori $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }

			registers[rt] = registers[rs] ^ signed_imm;

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x0f: //lui
			if (rs) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lui $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                        }

			registers[rt] = signed_imm << 16;
			registers[32] += 4;

                        free(num1);
                        free(num2);
                        break;

		case 0x0a: //slti
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "slti $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			
			if (registers[rs] < signed_imm){
				registers[rt] = 1;
			} else {
				registers[rt] = 0;
			}
			registers[32] += 4;
                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0b: //sltiu
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%u", rs);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sltiu $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }

			if (registers[rs] < (unsigned int) signed_imm){
                                registers[rt] = 1;
                        } else {
                                registers[rt] = 0;
                        }
                        registers[32] += 4;
                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x04: //beq
			t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "beq $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }
			if (registers[rs] == registers[rt]) {
                                registers[32] = registers[32] + 4 + (signed_imm << 2);
                        } else {
                                registers[32] += 4;
                        }

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x05: //bne
			t1 = asprintf(&num1, "%u", rs);
                        t2 = asprintf(&num2, "%u", rt);
                        t3 = asprintf(&num3, "%d", signed_imm);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "bne $"), num1);
                                strcat(strcat(assembly_string, ", $"), num2);
                                strcat(strcat(assembly_string, ", "), num3);
                        }

			if (registers[rs] != registers[rt]) {
				registers[32] = registers[32] + 4 + (signed_imm << 2);
			} else {
				registers[32] += 4;
			}
                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x20: //lb
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
			t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lb $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
				strcat(strcat(assembly_string, "($"), num3);
				strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
			free(num3);
			unknown_tf = 1;
                        break;

		case 0x24: //lbu
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lbu $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x21: //lh
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lh $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x25: //lhu
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lhu $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x23: //lw
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "lw $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }
			lw_val = 0;
			load_address = registers[rs]+signed_imm;

			for (int i=0; i<4; i++) {
				lw_val = lw_val << 8; 
				lw_val = lw_val | (int)data_memory[load_address + i-0x10000000];

			}
			registers[rt] = lw_val;
			registers[32] += 4;

			cache_tag = ( load_address >> 10) & 0x3fffff;
                        cache_index = (load_address >> 5) & 0x1f;

			// if location "index" of cache1 is valid
			// and location "index" of cache1 tag has a match => cache hit
			if (cache1_valid[cache_index] && cache1_tag[cache_index] == cache_tag) {
				*cache_hit +=1;
			} else {
				*cache_miss += 1;

				cache1_valid[cache_index] = 1;
				cache1_tag[cache_index] = cache_tag;
			}

			*cache_hit_miss_total += 1;

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x28: //sb
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sb $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x29: //sh
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sh $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

                        free(num1);
                        free(num2);
                        free(num3);
			unknown_tf = 1;
                        break;

		case 0x2b: //sw
			t1 = asprintf(&num1, "%u", rt);
                        t2 = asprintf(&num2, "%d", signed_imm);
                        t3 = asprintf(&num3, "%u", rs);
                        if (t1 == -1 || t2 == -1 || t3 == -1) {
                                perror("asprintf");
                                break;
                        } else {
                                strcat(strcpy(assembly_string, "sw $"), num1);
                                strcat(strcat(assembly_string, ", "), num2);
                                strcat(strcat(assembly_string, "($"), num3);
                                strcat(assembly_string, ")");
                        }

			load_address = registers[rs]+signed_imm;

			for (int i=0; i<4; i++) {
				temp = (registers[rt]>> 8 * (3-i)) & ((unsigned int)0xff);
				data_memory[load_address + i-0x10000000] = temp;

                        }
                        registers[32] += 4;

			cache_tag = ( load_address >> 10) & 0x3fffff;
                        cache_index = (load_address >> 5) & 0x1f;

                        // if location "index" of cache1 is valid
                        // and location "index" of cache1 tag has a match => cache hit
                        if (cache1_valid[cache_index] && cache1_tag[cache_index] == cache_tag) {
                                *cache_hit +=1;
				cache1_tag[cache_index] = cache_tag;
                        } else {
                                *cache_miss += 1;
                        }

                        *cache_hit_miss_total += 1;

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		default:
			strcpy(assembly_string, unknown);
			break;
	
	}

	return unknown_tf;
}

