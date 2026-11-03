// ----------------------------------------------------------------------------
// the type and dimension codes of cmmcomp ------------------------------------
// ----------------------------------------------------------------------------
// One numbering for the symbol table (symbol.type), an expression's value
// (expr.type) and a tree node (expr_node.type). The order matters: code
// compares with < and >, so a comp is above the reals and a function above
// every value (step P6 of #93).
// ----------------------------------------------------------------------------

#ifndef CMM_TYPES_H
#define CMM_TYPES_H

enum {
    TY_NONE   = 0,   // not declared yet; as an expression, void
    TY_INT    = 1,
    TY_FLOAT  = 2,
    TY_COMP   = 3,   // a comp variable, by its real half
    TY_IMAG   = 4,   // the imaginary half of a comp variable, <name>_i
    TY_CCONST = 5,   // a comp constant, 3+7.5i
    TY_FVOID  = 6,   // a function returning void, int, float, comp: 6 + its
    TY_FINT   = 7,   // return type (funcoes.c)
    TY_FFLOAT = 8,
    TY_FCOMP  = 9
};

// what a symbol is by its dimensions (symbol.isar): a scalar, a vector or a
// matrix; code compares with > AR_NONE for "an array"
enum {
    AR_NONE = 0,
    AR_1D   = 1,
    AR_2D   = 2
};

#endif // CMM_TYPES_H
