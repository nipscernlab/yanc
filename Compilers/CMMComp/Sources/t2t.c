// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include   <math.h>
#include <stdlib.h>
#include <string.h>

// local includes
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"

// ----------------------------------------------------------------------------
// data conversion ------------------------------------------------------------
// ----------------------------------------------------------------------------

// converts the integer x to a binary string of length w
// could be revised to support ints wider than 32 bits
char *itob(int x, int w)
{
	int z;
    char *b = (char *) malloc(w+1);
    b[0] = '\0';

	int s = (w > 31) ? 31 : w;
	if (w > 31)
    {
        for (z = 0; z < w- 31; z++)
            if (x < 0) strcat(b,"1");
            else       strcat(b,"0");
    }

    for (z = pow(2,s-1); z > 0; z >>= 1)
		strcat(b, ((x & z) == z) ? "1" : "0");

    return b;
}

// ----------------------------------------------------------------------------
// helper functions for accessing terminals -----------------------------------
// ----------------------------------------------------------------------------

// gets the id of the imag part of a complex var
// the real part is in the id parameter
int get_img_id(int id)
{
       char name[1024];
    sprintf(name, "%s_i", v_table[id].name);

       if (find_var(name) == -1) add_var(name);
    return find_var(name);
}

// splits the real and imaginary parts of a complex constant
// generating two floating-point entries in the table
void get_cmp_cst(expr e, expr *er, expr *ei)
{
    char  txt[64];
    float real, img;

    sscanf(v_table[e.id].name, "%f %f", &real, &img);

    sprintf(txt, "%f", real);
    *er = expr_make(2, exec_fnum(txt));

    sprintf(txt, "%f", img);
    *ei = expr_make(2, exec_fnum(txt));
}

// generates the extended float exprs for the real and imag parts
// of a complex number in memory
void get_cmp_ets(expr e, expr *er, expr *ei)
{
    *er = expr_make(2, e.id);
    *ei = expr_make(2, get_img_id(e.id));
}
