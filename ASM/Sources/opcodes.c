// ----------------------------------------------------------------------------
// tratamento de opcodes em assembly ------------------------------------------
// ----------------------------------------------------------------------------

#define NMNEMAX 999999 // usar array dinamico

// includes globais
#include <string.h>
#include  <stdio.h>

#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// variaveis locais -----------------------------------------------------------
// ----------------------------------------------------------------------------

int  u_count = 0;	      // contador de operacoes da ula
int  m_count = 0;         // contador de opcodes (parameter)
char m_name[NMNEMAX][64]; // nome     do opcode  (parameter)

// ----------------------------------------------------------------------------
// funcoes auxiliares ---------------------------------------------------------
// ----------------------------------------------------------------------------

// ve se um opcode ja foi usado
// se sim, pega o indice na tabela
// se nao, retorna -1
int find_opc(char *val)
{
	int ind = -1;

	for (int i = 0; i < m_count; i++)
	{
		if (strcmp(val, m_name[i]) == 0)
		{
			ind = i; break;
		}
	}

	return ind;
}

// ----------------------------------------------------------------------------
// funcoes de interface -------------------------------------------------------
// ----------------------------------------------------------------------------

char* opc_get(int i){return m_name[i];} // pega opcode do indice i
int   opc_cnt(     ){return m_count  ;} // pega numero de opcodes cadastrados
int   opc_ucnt(    ){return u_count  ;} // pega numero de operacoes da ula cadastrados

// addiciona um novo opcode na tabela
// o mne pode ser vazio (instrucoes que nao acrescentam recursos. ex: JMP)
void opc_add(char *mne)
{
	// se opcode ainda nao foi cadastrado...
	if ((find_opc(mne) == -1) && (strcmp(mne,"") != 0))
	{
		// --------------------------------------------------------------------
		// primeiro gera uma info dizendo que o recurso sera usado ------------
		// --------------------------------------------------------------------

		// nao tem mensagem pra   LOD ainda
		// nao tem mensagem pra P_LOD ainda

		if (strcmp(mne, "LDI") == 0)
		{
			// se ainda nao tem circuito de array handling, escreve a info
			if ((find_opc("LDI") == -1) && (find_opc("STI") == -1) &&
			    (find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_ARRAY_HANDLING);
			}
		}

		if (strcmp(mne, "ILI") == 0)
		{
			// se ainda nao tem circuito de array handling, escreve a info
			if ((find_opc("LDI") == -1) && (find_opc("STI") == -1) &&
			    (find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_ARRAY_HANDLING);
			}

			// se ainda nao tem circuito de inversao de bits, escreve a info
			if ((find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_BIT_REVERSE);
			}
		}

		// nao tem mensagem pra SET   ainda
		// nao tem mensagem pra SET_P ainda

		if (strcmp(mne, "STI") == 0)
		{
			// se ainda nao tem circuito de array handling, escreve a info
			if ((find_opc("LDI") == -1) && (find_opc("STI") == -1) &&
			    (find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_ARRAY_HANDLING);
			}
		}

		if (strcmp(mne, "ISI") == 0)
		{
			// se ainda nao tem circuito de array handling, escreve a info
			if ((find_opc("LDI") == -1) && (find_opc("STI") == -1) &&
			    (find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_ARRAY_HANDLING);
			}

			// se ainda nao tem circuito de inversao de bits, escreve a info
			if ((find_opc("ILI") == -1) && (find_opc("ISI") == -1))
			{
				printf(MSG_INFO_BIT_REVERSE);
			}
		}

		// nao tem mensagem pra PSH ainda
		// nao tem mensagem pra POP ainda

		if (strcmp(mne, "INN") == 0)
		{
			// se ainda nao tem circuito pra input handling, escreve a info
			if ((find_opc(  "INN") == -1) && (find_opc( "F_INN") == -1) &&
			    (find_opc("P_INN") == -1) && (find_opc("PF_INN") == -1))
			{
				printf(MSG_INFO_INPUT_HANDLING);
			}
		}

		if (strcmp(mne, "F_INN") == 0)
		{
			// se ainda nao tem circuito pra input handling, escreve a info
			if ((find_opc(  "INN") == -1) && (find_opc( "F_INN") == -1) &&
			    (find_opc("P_INN") == -1) && (find_opc("PF_INN") == -1))
			{
				printf(MSG_INFO_INPUT_HANDLING);
			}

			// se ainda nao tem circuito de int2float, escreve a info
			if ((find_opc("F_INN") == -1) && (find_opc("F_INN") == -1) &&
			    (find_opc(  "I2F") == -1) && (find_opc("I2F_M") == -1) && (find_opc("P_I2F_M") == -1))
			{
				printf(MSG_INFO_INT2FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "P_INN") == 0)
		{
			// se ainda nao tem circuito pra input handling, escreve a info
			if ((find_opc(  "INN") == -1) && (find_opc( "F_INN") == -1) &&
			    (find_opc("P_INN") == -1) && (find_opc("PF_INN") == -1))
			{
				printf(MSG_INFO_INPUT_HANDLING);
			}
		}

		if (strcmp(mne, "PF_INN") == 0)
		{
			// se ainda nao tem circuito pra input handling, escreve a info
			if ((find_opc(  "INN") == -1) && (find_opc( "F_INN") == -1) &&
			    (find_opc("P_INN") == -1) && (find_opc("PF_INN") == -1))
			{
				printf(MSG_INFO_INPUT_HANDLING);
			}

			// se ainda nao tem circuito de int2float, escreve a info
			if ((find_opc("F_INN") == -1) && (find_opc("F_INN") == -1) &&
			    (find_opc(  "I2F") == -1) && (find_opc("I2F_M") == -1) && (find_opc("P_I2F_M") == -1))
			{
				printf(MSG_INFO_INT2FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "OUT") == 0)
		{
			printf(MSG_INFO_OUTPUT_HANDLING);
		}

		// nao tem mensagem pra JMP ainda
		// nao tem mensagem pra JIZ ainda

		if (strcmp(mne, "CAL") == 0)
		{
			printf(MSG_INFO_STACK_MEMORY);
		}

		// nao tem mensagem pra RET ainda

		if (strcmp(mne, "ADD") == 0)
		{
			// se ainda nao tem circuito de soma pra int, escreve a info
			if ((find_opc("ADD") == -1) && (find_opc("S_ADD") == -1) && (find_opc("ADD_V") == -1))
			{
				printf(MSG_INFO_INT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "S_ADD") == 0)
		{
			// se ainda nao tem circuito de soma pra int, escreve a info
			if ((find_opc("ADD") == -1) && (find_opc("S_ADD") == -1) && (find_opc("ADD_V") == -1))
			{
				printf(MSG_INFO_INT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "F_ADD") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "SF_ADD") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "MLT") == 0)
		{
			// se ainda nao tem circuito de multiplicador inteiro, escreve a info
			if ((find_opc("MLT") == -1) && (find_opc("S_MLT") == -1) && (find_opc("MLT_V") == -1))
			{
				printf(MSG_INFO_INT_MULT); u_count++;
			}
		}

		if (strcmp(mne, "S_MLT") == 0)
		{
			// se ainda nao tem circuito de multiplicador inteiro, escreve a info
			if ((find_opc("MLT") == -1) && (find_opc("S_MLT") == -1) && (find_opc("MLT_V") == -1))
			{
				printf(MSG_INFO_INT_MULT); u_count++;
			}
		}

		if (strcmp(mne, "F_MLT") == 0)
		{
			// se ainda nao tem circuito de multiplicador float-point, escreve a info
			if ((find_opc("F_MLT") == -1) && (find_opc("SF_MLT") == -1) && (find_opc("F_MLT_V") == -1))
			{
				printf(MSG_INFO_FLOAT_MULT); u_count++;
			}
		}

		if (strcmp(mne, "SF_MLT") == 0)
		{
			// se ainda nao tem circuito de multiplicador float-point, escreve a info
			if ((find_opc("F_MLT") == -1) && (find_opc("SF_MLT") == -1) && (find_opc("F_MLT_V") == -1))
			{
				printf(MSG_INFO_FLOAT_MULT); u_count++;
			}
		}

		if (strcmp(mne, "DIV") == 0)
		{
			// se ainda nao tem circuito de divisor inteiro, escreve a info
			if ((find_opc("DIV") == -1) && (find_opc("S_DIV") == -1))
			{
				printf(MSG_INFO_INT_DIV); u_count++;
			}
		}

		if (strcmp(mne, "S_DIV") == 0)
		{
			// se ainda nao tem circuito de divisor inteiro, escreve a info
			if ((find_opc("DIV") == -1) && (find_opc("S_DIV") == -1))
			{
				printf(MSG_INFO_INT_DIV); u_count++;
			}
		}

		if (strcmp(mne, "F_DIV") == 0)
		{
			// se ainda nao tem circuito de divisor float-point, escreve a info
			if ((find_opc("F_DIV") == -1) && (find_opc("SF_DIV") == -1))
			{
				printf(MSG_INFO_FLOAT_DIV); u_count++;
			}
		}

		if (strcmp(mne, "SF_DIV") == 0)
		{
			// se ainda nao tem circuito de divisor float-point, escreve a info
			if ((find_opc("F_DIV") == -1) && (find_opc("SF_DIV") == -1))
			{
				printf(MSG_INFO_FLOAT_DIV); u_count++;
			}
		}

		if (strcmp(mne, "MOD") == 0)
		{
			// se ainda nao tem circuito de modulo inteiro, escreve a info
			if ((find_opc("MOD") == -1) && (find_opc("S_MOD") == -1))
			{
				printf(MSG_INFO_MODULO); u_count++;
			}
		}

		if (strcmp(mne, "S_MOD") == 0)
		{
			// se ainda nao tem circuito de modulo inteiro, escreve a info
			if ((find_opc("MOD") == -1) && (find_opc("S_MOD") == -1))
			{
				printf(MSG_INFO_MODULO); u_count++;
			}
		}

		if (strcmp(mne, "SGN") == 0)
		{
			// se ainda nao tem o circuito de sinal inteiro, escreve a info
			if ((find_opc("SGN") == -1) && (find_opc("S_SGN") == -1))
			{
				printf(MSG_INFO_INT_SIGN); u_count++;
			}
		}

		if (strcmp(mne, "S_SGN") == 0)
		{
			// se ainda nao tem o circuito de sinal inteiro, escreve a info
			if ((find_opc("SGN") == -1) && (find_opc("S_SGN") == -1))
			{
				printf(MSG_INFO_INT_SIGN); u_count++;
			}
		}

		if (strcmp(mne, "F_SGN") == 0)
		{
			// se ainda nao tem o circuito de sinal float-point, escreve a info
			if ((find_opc("F_SGN") == -1) && (find_opc("SF_SGN") == -1))
			{
				printf(MSG_INFO_FLOAT_SIGN); u_count++;
			}
		}

		if (strcmp(mne, "SF_SGN") == 0)
		{
			// se ainda nao tem o circuito de sinal float-point, escreve a info
			if ((find_opc("F_SGN") == -1) && (find_opc("SF_SGN") == -1))
			{
				printf(MSG_INFO_FLOAT_SIGN); u_count++;
			}
		}

		if (strcmp(mne, "NEG") == 0)
		{
			// se ainda nao tem circuito de negativo inteiro, escreve a info
			if ((find_opc("NEG") == -1) && (find_opc("NEG_M") == -1) && (find_opc("P_NEG_M") == -1))
			{
				printf(MSG_INFO_INT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "NEG_M") == 0)
		{
			// se ainda nao tem circuito de negativo inteiro, escreve a info
			if ((find_opc("NEG") == -1) && (find_opc("NEG_M") == -1) && (find_opc("P_NEG_M") == -1))
			{
				printf(MSG_INFO_INT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "P_NEG_M") == 0)
		{
			// se ainda nao tem circuito de negativo inteiro, escreve a info
			if ((find_opc("NEG") == -1) && (find_opc("NEG_M") == -1) && (find_opc("P_NEG_M") == -1))
			{
				printf(MSG_INFO_INT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "F_NEG") == 0)
		{
			// se ainda nao tem o circuito de negativo float-point, escreve a info
			if ((find_opc("F_NEG") == -1) && (find_opc("F_NEG_M") == -1) && (find_opc("PF_NEG_M") == -1))
			{
				printf(MSG_INFO_FLOAT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "F_NEG_M") == 0)
		{
			// se ainda nao tem o circuito de negativo float-point, escreve a info
			if ((find_opc("F_NEG") == -1) && (find_opc("F_NEG_M") == -1) && (find_opc("PF_NEG_M") == -1))
			{
				printf(MSG_INFO_FLOAT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "PF_NEG_M") == 0)
		{
			// se ainda nao tem o circuito de negativo float-point, escreve a info
			if ((find_opc("F_NEG") == -1) && (find_opc("F_NEG_M") == -1) && (find_opc("PF_NEG_M") == -1))
			{
				printf(MSG_INFO_FLOAT_NEG); u_count++;
			}
		}

		if (strcmp(mne, "ABS") == 0)
		{
			// se ainda nao tem o circuito de absoluto inteiro, escreve a info
			if ((find_opc("ABS") == -1) && (find_opc("ABS_M") == -1) && (find_opc("P_ABS_M") == -1))
			{
				printf(MSG_INFO_INT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "ABS_M") == 0)
		{
			// se ainda nao tem o circuito de absoluto inteiro, escreve a info
			if ((find_opc("ABS") == -1) && (find_opc("ABS_M") == -1) && (find_opc("P_ABS_M") == -1))
			{
				printf(MSG_INFO_INT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "P_ABS_M") == 0)
		{
			// se ainda nao tem o circuito de absoluto inteiro, escreve a info
			if ((find_opc("ABS") == -1) && (find_opc("ABS_M") == -1) && (find_opc("P_ABS_M") == -1))
			{
				printf(MSG_INFO_INT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "F_ABS") == 0)
		{
			// se ainda nao tem o circuito de absoluto float-point, escreve a info
			if ((find_opc("F_ABS") == -1) && (find_opc("F_ABS_M") == -1) && (find_opc("PF_ABS_M") == -1))
			{
				printf(MSG_INFO_FLOAT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "F_ABS_M") == 0)
		{
			// se ainda nao tem o circuito de absoluto float-point, escreve a info
			if ((find_opc("F_ABS") == -1) && (find_opc("F_ABS_M") == -1) && (find_opc("PF_ABS_M") == -1))
			{
				printf(MSG_INFO_FLOAT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "PF_ABS_M") == 0)
		{
			// se ainda nao tem o circuito de absoluto float-point, escreve a info
			if ((find_opc("F_ABS") == -1) && (find_opc("F_ABS_M") == -1) && (find_opc("PF_ABS_M") == -1))
			{
				printf(MSG_INFO_FLOAT_ABS); u_count++;
			}
		}

		if (strcmp(mne, "PST") == 0)
		{
			// se ainda nao tem o circuito de pset pra int, escreve a info
			if ((find_opc("PST") == -1) && (find_opc("PST_M") == -1) && (find_opc("P_PST_M") == -1))
			{
				printf(MSG_INFO_INT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "PST_M") == 0)
		{
			// se ainda nao tem o circuito de pset pra int, escreve a info
			if ((find_opc("PST") == -1) && (find_opc("PST_M") == -1) && (find_opc("P_PST_M") == -1))
			{
				printf(MSG_INFO_INT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "P_PST_M") == 0)
		{
			// se ainda nao tem o circuito de pset pra int, escreve a info
			if ((find_opc("PST") == -1) && (find_opc("PST_M") == -1) && (find_opc("P_PST_M") == -1))
			{
				printf(MSG_INFO_INT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "F_PST") == 0)
		{
			// se ainda nao tem o circuito de pset pra float-point, escreve a info
			if ((find_opc("F_PST") == -1) && (find_opc("F_PST_M") == -1) && (find_opc("PF_PST_M") == -1))
			{
				printf(MSG_INFO_FLOAT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "F_PST_M") == 0)
		{
			// se ainda nao tem o circuito de pset pra float-point, escreve a info
			if ((find_opc("F_PST") == -1) && (find_opc("F_PST_M") == -1) && (find_opc("PF_PST_M") == -1))
			{
				printf(MSG_INFO_FLOAT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "PF_PST_M") == 0)
		{
			// se ainda nao tem o circuito de pset pra float-point, escreve a info
			if ((find_opc("F_PST") == -1) && (find_opc("F_PST_M") == -1) && (find_opc("PF_PST_M") == -1))
			{
				printf(MSG_INFO_FLOAT_PSET); u_count++;
			}
		}

		if (strcmp(mne, "NRM") == 0)
		{
			// se ainda nao tem o circuito de normalizacao, escreve a info
			if ((find_opc("NRM") == -1) && (find_opc("NRM_M") == -1) && (find_opc("P_NRM_M") == -1))
			{
				printf(MSG_INFO_NORM); u_count++;
			}
		}

		if (strcmp(mne, "NRM_M") == 0)
		{
			// se ainda nao tem o circuito de normalizacao, escreve a info
			if ((find_opc("NRM") == -1) && (find_opc("NRM_M") == -1) && (find_opc("P_NRM_M") == -1))
			{
				printf(MSG_INFO_NORM); u_count++;
			}
		}

		if (strcmp(mne, "P_NRM_M") == 0)
		{
			// se ainda nao tem o circuito de normalizacao, escreve a info
			if ((find_opc("NRM") == -1) && (find_opc("NRM_M") == -1) && (find_opc("P_NRM_M") == -1))
			{
				printf(MSG_INFO_NORM); u_count++;
			}
		}

		if (strcmp(mne, "I2F") == 0)
		{
			// se ainda nao tem circuito de int2float, escreve a info
			if ((find_opc("F_INN") == -1) && (find_opc("F_INN") == -1) &&
			    (find_opc(  "I2F") == -1) && (find_opc("I2F_M") == -1) && (find_opc("P_I2F_M") == -1))
			{
				printf(MSG_INFO_INT2FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "I2F_M") == 0)
		{
			// se ainda nao tem circuito de int2float, escreve a info
			if ((find_opc("F_INN") == -1) && (find_opc("F_INN") == -1) &&
			    (find_opc(  "I2F") == -1) && (find_opc("I2F_M") == -1) && (find_opc("P_I2F_M") == -1))
			{
				printf(MSG_INFO_INT2FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "P_I2F_M") == 0)
		{
			// se ainda nao tem circuito de int2float, escreve a info
			if ((find_opc("F_INN") == -1) && (find_opc("F_INN") == -1) &&
			    (find_opc(  "I2F") == -1) && (find_opc("I2F_M") == -1) && (find_opc("P_I2F_M") == -1))
			{
				printf(MSG_INFO_INT2FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "F2I") == 0)
		{
			// se ainda nao tem o circuito de float2int, escreve a info
			if ((find_opc("F2I") == -1) && (find_opc("F2I_M") == -1) && (find_opc("P_F2I_M") == -1))
			{
				printf(MSG_INFO_FLOAT2INT); u_count++;
			}
		}

		if (strcmp(mne, "F2I_M") == 0)
		{
			// se ainda nao tem o circuito de float2int, escreve a info
			if ((find_opc("F2I") == -1) && (find_opc("F2I_M") == -1) && (find_opc("P_F2I_M") == -1))
			{
				printf(MSG_INFO_FLOAT2INT); u_count++;
			}
		}

		if (strcmp(mne, "P_F2I_M") == 0)
		{
			// se ainda nao tem o circuito de float2int, escreve a info
			if ((find_opc("F2I") == -1) && (find_opc("F2I_M") == -1) && (find_opc("P_F2I_M") == -1))
			{
				printf(MSG_INFO_FLOAT2INT); u_count++;
			}
		}

		if (strcmp(mne, "AND") == 0)
		{
			// se ainda nao tem o circuito de and, escreve a info
			if ((find_opc("AND") == -1) && (find_opc("S_AND") == -1))
			{
				printf(MSG_INFO_OP_AND); u_count++;
			}
		}

		if (strcmp(mne, "S_AND") == 0)
		{
			// se ainda nao tem o circuito de and, escreve a info
			if ((find_opc("AND") == -1) && (find_opc("S_AND") == -1))
			{
				printf(MSG_INFO_OP_AND); u_count++;
			}
		}

		if (strcmp(mne, "ORR") == 0)
		{
			// se ainda nao tem o circuito de or, escreve a info
			if ((find_opc("ORR") == -1) && (find_opc("S_ORR") == -1))
			{
				printf(MSG_INFO_OP_OR); u_count++;
			}
		}

		if (strcmp(mne, "S_ORR") == 0)
		{
			// se ainda nao tem o circuito de or, escreve a info
			if ((find_opc("ORR") == -1) && (find_opc("S_ORR") == -1))
			{
				printf(MSG_INFO_OP_OR); u_count++;
			}
		}

		if (strcmp(mne, "XOR") == 0)
		{
			// se ainda nao tem o circuito de xor, escreve a info
			if ((find_opc("XOR") == -1) && (find_opc("S_XOR") == -1))
			{
				printf(MSG_INFO_OP_XOR); u_count++;
			}
		}

		if (strcmp(mne, "S_XOR") == 0)
		{
			// se ainda nao tem o circuito de xor, escreve a info
			if ((find_opc("XOR") == -1) && (find_opc("S_XOR") == -1))
			{
				printf(MSG_INFO_OP_XOR); u_count++;
			}
		}

		if (strcmp(mne, "INV") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("INV") == -1) && (find_opc("INV_M") == -1) && (find_opc("P_INV_M") == -1))
			{
				printf(MSG_INFO_OP_NOT); u_count++;
			}
		}

		if (strcmp(mne, "INV_M") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("INV") == -1) && (find_opc("INV_M") == -1) && (find_opc("P_INV_M") == -1))
			{
				printf(MSG_INFO_OP_NOT); u_count++;
			}
		}

		if (strcmp(mne, "P_INV_M") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("INV") == -1) && (find_opc("INV_M") == -1) && (find_opc("P_INV_M") == -1))
			{
				printf(MSG_INFO_OP_NOT); u_count++;
			}
		}

		if (strcmp(mne, "LAN") == 0)
		{
			// se ainda nao tem o circuito de and, escreve a info
			if ((find_opc("LAN") == -1) && (find_opc("S_LAN") == -1))
			{
				printf(MSG_INFO_OP_LAND); u_count++;
			}
		}

		if (strcmp(mne, "S_LAN") == 0)
		{
			// se ainda nao tem o circuito de and, escreve a info
			if ((find_opc("LAN") == -1) && (find_opc("S_LAN") == -1))
			{
				printf(MSG_INFO_OP_LAND); u_count++;
			}
		}

		if (strcmp(mne, "LOR") == 0)
		{
			// se ainda nao tem o circuito de or, escreve a info
			if ((find_opc("LOR") == -1) && (find_opc("S_LOR") == -1))
			{
				printf(MSG_INFO_OP_LOR); u_count++;
			}
		}

		if (strcmp(mne, "S_LOR") == 0)
		{
			// se ainda nao tem o circuito de or, escreve a info
			if ((find_opc("LOR") == -1) && (find_opc("S_LOR") == -1))
			{
				printf(MSG_INFO_OP_LOR); u_count++;
			}
		}

		if (strcmp(mne, "LIN") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("LIN") == -1) && (find_opc("LIN_M") == -1) && (find_opc("P_LIN_M") == -1))
			{
				printf(MSG_INFO_OP_LNOT); u_count++;
			}
		}

		if (strcmp(mne, "LIN_M") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("LIN") == -1) && (find_opc("LIN_M") == -1) && (find_opc("P_LIN_M") == -1))
			{
				printf(MSG_INFO_OP_LNOT); u_count++;
			}
		}

		if (strcmp(mne, "P_LIN_M") == 0)
		{
			// se ainda nao tem o circuito de not, escreve a info
			if ((find_opc("LIN") == -1) && (find_opc("LIN_M") == -1) && (find_opc("P_LIN_M") == -1))
			{
				printf(MSG_INFO_OP_LNOT); u_count++;
			}
		}

		if (strcmp(mne, "LES") == 0)
		{
			// se ainda nao tem o circuito de menor para int, escreve a info
			if ((find_opc("LES") == -1) && (find_opc("S_LES") == -1))
			{
				printf(MSG_INFO_OP_LES_INT); u_count++;
			}
		}

		if (strcmp(mne, "S_LES") == 0)
		{
			// se ainda nao tem o circuito de menor para int, escreve a info
			if ((find_opc("LES") == -1) && (find_opc("S_LES") == -1))
			{
				printf(MSG_INFO_OP_LES_INT); u_count++;
			}
		}

		if (strcmp(mne, "F_LES") == 0)
		{
			// se ainda nao tem o circuito de menor para float, escreve a info
			if ((find_opc("F_LES") == -1) && (find_opc("SF_LES") == -1))
			{
				printf(MSG_INFO_OP_LES_FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "SF_LES") == 0)
		{
			// se ainda nao tem o circuito de menor para float, escreve a info
			if ((find_opc("F_LES") == -1) && (find_opc("SF_LES") == -1))
			{
				printf(MSG_INFO_OP_LES_FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "GRE") == 0)
		{
			// se ainda nao tem o circuito de maior para int, escreve a info
			if ((find_opc("GRE") == -1) && (find_opc("S_GRE") == -1))
			{
				printf(MSG_INFO_OP_GRE_INT); u_count++;
			}
		}

		if (strcmp(mne, "S_GRE") == 0)
		{
			// se ainda nao tem o circuito de maior para int, escreve a info
			if ((find_opc("GRE") == -1) && (find_opc("S_GRE") == -1))
			{
				printf(MSG_INFO_OP_GRE_INT); u_count++;
			}
		}

		if (strcmp(mne, "F_GRE") == 0)
		{
			// se ainda nao tem o circuito de maior para float, escreve a info
			if ((find_opc("F_GRE") == -1) && (find_opc("SF_GRE") == -1))
			{
				printf(MSG_INFO_OP_GRE_FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "SF_GRE") == 0)
		{
			// se ainda nao tem o circuito de maior para float, escreve a info
			if ((find_opc("F_GRE") == -1) && (find_opc("SF_GRE") == -1))
			{
				printf(MSG_INFO_OP_GRE_FLOAT); u_count++;
			}
		}

		if (strcmp(mne, "EQU") == 0)
		{
			// se ainda nao tem o circuito de igual, escreve a info
			if ((find_opc("EQU") == -1) && (find_opc("S_EQU") == -1))
			{
				printf(MSG_INFO_OP_EQU); u_count++;
			}
		}

		if (strcmp(mne, "S_EQU") == 0)
		{
			// se ainda nao tem o circuito de igual, escreve a info
			if ((find_opc("EQU") == -1) && (find_opc("S_EQU") == -1))
			{
				printf(MSG_INFO_OP_EQU); u_count++;
			}
		}

		if (strcmp(mne, "SHL") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a esquerda, escreve a info
			if ((find_opc("SHL") == -1) && (find_opc("S_SHL") == -1))
			{
				printf(MSG_INFO_OP_SHL); u_count++;
			}
		}

		if (strcmp(mne, "S_SHL") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a esquerda, escreve a info
			if ((find_opc("SHL") == -1) && (find_opc("S_SHL") == -1))
			{
				printf(MSG_INFO_OP_SHL); u_count++;
			}
		}

		if (strcmp(mne, "SHR") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a direita, escreve a info
			if ((find_opc("SHR") == -1) && (find_opc("S_SHR") == -1))
			{
				printf(MSG_INFO_OP_SHR); u_count++;
			}
		}

		if (strcmp(mne, "S_SHR") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a direita, escreve a info
			if ((find_opc("SHR") == -1) && (find_opc("S_SHR") == -1))
			{
				printf(MSG_INFO_OP_SHR); u_count++;
			}
		}

		if (strcmp(mne, "SRS") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a direita com sinal, escreve a info
			if ((find_opc("SRS") == -1) && (find_opc("S_SRS") == -1))
			{
				printf(MSG_INFO_OP_SRS); u_count++;
			}
		}

		if (strcmp(mne, "S_SRS") == 0)
		{
			// se ainda nao tem o circuito de deslocamento para a direita com sinal, escreve a info
			if ((find_opc("SRS") == -1) && (find_opc("S_SRS") == -1))
			{
				printf(MSG_INFO_OP_SRS); u_count++;
			}
		}

		// nao tem mensagem pra NOP

		if (strcmp(mne, "F_ROT") == 0)
		{
			// se ainda nao tem o opcode F_ROT, escreve a info
			if (find_opc("F_ROT") == -1)
			{
				printf(MSG_INFO_FLOAT_SQRT); u_count++;
			}
		}

		if (strcmp(mne, "F_SU1") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "F_SU2") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "SF_SU1") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "SF_SU2") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		// nao tem mensagem pra   LOD_V
		// nao tem mensagem pra P_LOD_V
		// nao tem mensagem pra   SET_V

		if (strcmp(mne, "ADD_V") == 0)
		{
			// se ainda nao tem circuito de soma pra int, escreve a info
			if ((find_opc("ADD") == -1) && (find_opc("S_ADD") == -1) && (find_opc("ADD_V") == -1))
			{
				printf(MSG_INFO_INT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "F_ADD_V") == 0)
		{
			// se ainda nao tem circuito de soma pra float, escreve a info
			if ((find_opc( "F_ADD") == -1) && (find_opc("SF_ADD") == -1) && (find_opc("F_ADD_V") == -1) &&
			    (find_opc( "F_SU1") == -1) && (find_opc( "F_SU2") == -1) &&
				(find_opc("SF_SU1") == -1) && (find_opc("SF_SU2") == -1))
			{
				printf(MSG_INFO_FLOAT_ADDER); u_count++;
			}
		}

		if (strcmp(mne, "MLT_V") == 0)
		{
			// se ainda nao tem circuito de multiplicador inteiro, escreve a info
			if ((find_opc("MLT") == -1) && (find_opc("S_MLT") == -1) && (find_opc("MLT_V") == -1))
			{
				printf(MSG_INFO_INT_MULT); u_count++;
			}
		}

		if (strcmp(mne, "F_MLT_V") == 0)
		{
			// se ainda nao tem circuito de multiplicador float-point, escreve a info
			if ((find_opc("F_MLT") == -1) && (find_opc("SF_MLT") == -1) && (find_opc("F_MLT_V") == -1))
			{
				printf(MSG_INFO_FLOAT_MULT); u_count++;
			}
		}

		// --------------------------------------------------------------------
		// depois cadastra o opcode novo --------------------------------------
		// --------------------------------------------------------------------
		
    	strcpy(m_name[m_count], mne); m_count++;
	}
}

// verifica se tem instrucao INN
int opc_inn() {return (find_opc("INN") != -1) | (find_opc("P_INN") != -1) | (find_opc("F_INN") != -1) | (find_opc("PF_INN") != -1);}
// verifica se tem instrucao OUT
int opc_out() {return  find_opc("OUT") != -1;}
// verifica se tem instrucao CAL
int opc_cal() {return  find_opc("CAL") != -1;}