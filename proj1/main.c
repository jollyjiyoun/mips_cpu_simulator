#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>

char * opcode (unsigned int cur_buf, char * assembly_string);
char * rType (unsigned int cur_buf, char * assembly_string);
char * iType (unsigned int cur_buf, char * assembly_string);

int main(int argc, char** argv){
	// printf("%s\n", argv[1]);
	FILE *fp;
	unsigned int * buffer = NULL;
	size_t len = 0;
	ssize_t read;
	char * assembly_string = malloc (sizeof(char) * 30);
	char * print_instruction = malloc(sizeof(char) * 60);
	char * buf_string;
	char * iteration;

	fp = fopen(argv[1], "rb");
	if(fp == NULL)
		exit(EXIT_FAILURE);
	
	/* seek end of file, count length of file, and rewind file to start */
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	/* allocate buffer memory based on file length retrieved above */
	buffer = (unsigned int*)malloc(sizeof(unsigned int) * len);

	if (buffer == NULL) {
		exit(EXIT_FAILURE);
	}

	/* read File fp data 4 bytes(32 bits) each until file length(len), and append to buffer */
	read = fread(buffer, 4, len, fp);

	/* display data in buffer */
	for (size_t j = 0; j < read; j++) {
		// fprintf(stdout, "before: %08x\n", buffer[j]);
		buffer[j] = __bswap_32(buffer[j]);
		//printf("before opcode\n");
		opcode(buffer[j], assembly_string);
		
		if (asprintf(&iteration, "%ld", j) == -1 || asprintf(&buf_string, "%08x", buffer[j]) == -1) {
			       perror("asprintf");
			       break;
		}	       

		else {
			strcat(strcpy(print_instruction, "inst "), iteration);
			strcat(print_instruction, ": ");
			strcat(print_instruction, buf_string);
			strcat(print_instruction, " ");
			strcat(print_instruction, assembly_string);

			printf("%s\n", print_instruction);

			free(iteration);
			free(buf_string);

			* assembly_string = '\0';
		}
		iteration = NULL;
		buf_string = NULL;

	}

	/* while((read = getline(&buffer, &len, fp)) != -1) {
		printf("Retrieved line of length %zu:\n", read);
		printf("%s\n", buffer);
	} */
	
	/* close File fp */
	fclose(fp);

	/* free heap memory */
	free(buffer);

	free(print_instruction);
		
	free(assembly_string);

	exit(EXIT_SUCCESS);
}

char * opcode (unsigned int cur_buf, char * assembly_string) {
	
	unsigned int opcode_val;
	int jtype_target;
	int t1;
	char * num1;
	
	/* get opcode value by shift left 26 bits -> mask right 6 bits */
	opcode_val = (cur_buf >> 26) & 127;
	
	//printf("%02x\n", opcode_val);	
	switch (opcode_val) {
		case 0:
			//printf("in opcode case 0\n");
			assembly_string = rType(cur_buf, assembly_string);
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
                        free(num1);
                        break;

		default:
			assembly_string = iType(cur_buf, assembly_string);
			break;
	}

	return assembly_string;
}

char * rType (unsigned int cur_buf, char * assembly_string) {
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
			free(num1);
                        free(num2);
                        free(num3);
                        break;

		case 0x0c: //syscall
			if (rs_rt_rd_shamt) {
                                strcpy(assembly_string, unknown);
                                break;
                        }
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

	return assembly_string;
}


char * iType (unsigned int cur_buf, char * assembly_string) {

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

                        free(num1);
                        free(num2);
                        free(num3);
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

                        free(num1);
                        free(num2);
                        free(num3);
                        break;

		default:
			strcpy(assembly_string, unknown);
			break;
	
	}

	return assembly_string;
}




