#include "c.h"

static char prec[] = {
#define xx(a, b, c, d, e, f, g) c,
#define yy(a, b, c, d, e, f, g) c,
#include "token.h"
};

static int oper[] = {
#define xx(a, b, c, d, e, f, g) d,
#define yy(a, b, c, d, e, f, g) d,
#include "token.h"
};

float refinc = 1.0;
static Tree expr2(void);
static Tree expr3(int);
static Tree nullcheck(Tree);
static Tree postfix(Tree);
static Tree unary(void);
static Tree primary(void);
static Type super(Type ty);

static Type super(Type ty)
{
    switch (ty->op) {
    case INT:
        if (ty->size < inttype->size)
            return inttype;
        break;
    case UNSIGNED:
        if (ty->size < unsignedtype->size)
            return unsignedtype;
        break;
    case POINTER:
        return unsignedptr;
    }
    return ty;
}

Tree expr(int tok)
{
    static char stop[] = { IF, ID, '}', 0 };
    Tree p             = expr1(0);

    while (curtok == ',') {
        Tree q;
        curtok = gettok();
        q = pointer(expr1(0));
        p = tree(RIGHT, q->type, root(value(p)), q);
    }
    if (tok)
        test(tok, stop);
    return p;
}

Tree expr0(int tok)
{
    return root(expr(tok));
}

Tree expr1(int tok)
{
    static char stop[] = { IF, ID, 0 };
    Tree p             = expr2();

    if (curtok == '=' || (prec[curtok] >= 6 && prec[curtok] <= 8) || (prec[curtok] >= 11 && prec[curtok] <= 13)) {
        int op = curtok;
        curtok = gettok();
        if (oper[op] == ASGN)
            p = asgntree(ASGN, p, value(expr1(0)));
        else {
            expect('=');
            p = incr(op, p, expr1(0));
        }
    }
    if (tok)
        test(tok, stop);
    return p;
}

Tree incr(int op, Tree v, Tree e)
{
    return asgntree(ASGN, v, (*optree[op])(oper[op], v, e));
}

static Tree expr2(void)
{
    Tree p = expr3(4);

    if (curtok == '?') {
        Tree l, r;
        Coordinate pts[2];
        if (Aflag > 1 && isfunc(p->type))
            warning("%s used in a conditional expression\n", funcname(p));
        p      = pointer(p);
        curtok = gettok();
        pts[0] = src;
        l      = pointer(expr(':'));
        pts[1] = src;
        r      = pointer(expr2());
        if (generic(p->op) != CNST && events.points) {
            apply(events.points, &pts[0], &l);
            apply(events.points, &pts[1], &r);
        }
        p = condtree(p, l, r);
    }
    return p;
}

Tree value(Tree p)
{
    int op = generic(rightkid(p)->op);

    if (p->type != voidtype && (op == AND || op == OR || op == NOT || op == EQ || op == NE ||
                                op == LE || op == LT || op == GE || op == GT))
        p = condtree(p, consttree(1, inttype), consttree(0, inttype));
    return p;
}

static Tree expr3(int k)
{
    int k1;
    Tree p = unary();

    for (k1 = prec[curtok]; k1 >= k; k1--)
        while (prec[curtok] == k1 && *incp != '=') {
            Tree r;
            Coordinate pt;
            int op = curtok;
            curtok = gettok();
            pt     = src;
            p      = pointer(p);
            if (op == ANDAND || op == OROR) {
                r = pointer(expr3(k1));
                if (events.points)
                    apply(events.points, &pt, &r);
            } else
                r = pointer(expr3(k1 + 1));
            p = (*optree[op])(oper[op], p, r);
        }
    return p;
}

static Tree unary(void)
{
    Tree p;

    switch (curtok) {
    case '*':
        curtok = gettok();
        p = unary();
        p = pointer(p);
        if (isptr(p->type) && (isfunc(p->type->type) || isarray(p->type->type)))
            p = retype(p, p->type->type);
        else {
            if (YYnull)
                p = nullcheck(p);
            p = rvalue(p);
        }
        break;
    case '&':
        curtok = gettok();
        p = unary();
        if (isarray(p->type) || isfunc(p->type))
            p = retype(p, ptr(p->type));
        else
            p = lvalue(p);
        if (isaddrop(p->op) && p->u.sym->sclass == REGISTER)
            error("invalid operand of unary &; `%s' is declared register\n", p->u.sym->name);

        else if (isaddrop(p->op))
            p->u.sym->addressed = 1;
        break;
    case '+':
        curtok = gettok();
        p = unary();
        p = pointer(p);
        if (isarith(p->type))
            p = cast(p, promote(p->type));
        else
            typeerror(ADD, p, NULL);
        break;
    case '-':
        curtok = gettok();
        p = unary();
        p = pointer(p);
        if (isarith(p->type)) {
            Type ty = promote(p->type);
            p       = cast(p, ty);
            if (isunsigned(ty)) {
                warning("unsigned operand of unary -\n");
                p = simplify(ADD, ty, simplify(BCOM, ty, p, NULL), cnsttree(ty, 1UL));
            } else
                p = simplify(NEG, ty, p, NULL);
        } else
            typeerror(SUB, p, NULL);
        break;
    case '~':
        curtok = gettok();
        p = unary();
        p = pointer(p);
        if (isint(p->type)) {
            Type ty = promote(p->type);
            p       = simplify(BCOM, ty, cast(p, ty), NULL);
        } else
            typeerror(BCOM, p, NULL);
        break;
    case '!':
        curtok = gettok();
        p = unary();
        p = pointer(p);
        if (isscalar(p->type))
            p = simplify(NOT, inttype, cond(p), NULL);
        else
            typeerror(NOT, p, NULL);
        break;
    case INCR:
        curtok = gettok();
        p = unary();
        p = incr(INCR, pointer(p), consttree(1, inttype));
        break;
    case DECR:
        curtok = gettok();
        p = unary();
        p = incr(DECR, pointer(p), consttree(1, inttype));
        break;
    case TYPECODE:
    case SIZEOF: {
        int op = curtok;
        Type ty;
        p = NULL;
        curtok = gettok();
        if (curtok == '(') {
            curtok = gettok();
            if (istypename(curtok, tsym)) {
                ty = typename();
                expect(')');
            } else {
                p  = postfix(expr(')'));
                ty = p->type;
            }
        } else {
            p  = unary();
            ty = p->type;
        }
        assert(ty);
        if (op == TYPECODE)
            p = cnsttree(inttype, (long)ty->op);
        else {
            if (isfunc(ty) || ty->size == 0)
                error("invalid type argument `%t' to `sizeof'\n", ty);
            else if (p && rightkid(p)->op == FIELD)
                error("`sizeof' applied to a bit field\n");
            p = cnsttree(unsignedlong, (unsigned long)ty->size);
        }
    } break;
    case '(':
        curtok = gettok();
        if (istypename(curtok, tsym)) {
            Type ty, ty1 = typename(), pty;
            expect(')');
            ty = unqual(ty1);
            if (isenum(ty)) {
                Type ty2 = ty->type;
                if (isconst(ty1))
                    ty2 = qual(CONST, ty2);
                if (isvolatile(ty1))
                    ty2 = qual(VOLATILE, ty2);
                ty1 = ty2;
                ty  = ty->type;
            }
            p   = pointer(unary());
            pty = p->type;
            if (isenum(pty))
                pty = pty->type;
            if ((isarith(pty) && isarith(ty)) ||
                (isptr(pty) && isptr(ty))) {
                explicitCast++;
                p = cast(p, ty);
                explicitCast--;
            } else if ((isptr(pty) && isint(ty)) ||
                       (isint(pty) && isptr(ty))) {
                if (Aflag >= 1 && ty->size < pty->size)
                    warning("conversion from `%t' to `%t' is compiler dependent\n", p->type, ty);

                p = cast(p, ty);
            } else if (ty != voidtype) {
                error("cast from `%t' to `%t' is illegal\n", p->type, ty1);
                ty1 = inttype;
            }
            if (generic(p->op) == INDIR || ty->size == 0)
                p = tree(RIGHT, ty1, NULL, p);
            else
                p = retype(p, ty1);
        } else
            p = postfix(expr(')'));
        break;
    default:
        p = postfix(primary());
    }
    return p;
}

static Tree postfix(Tree p)
{
    for (;;)
        switch (curtok) {
        case INCR:
            p = tree(RIGHT, p->type, tree(RIGHT, p->type, p, incr(curtok, p, consttree(1, inttype))), p);
            curtok = gettok();
            break;
        case DECR:
            p = tree(RIGHT, p->type, tree(RIGHT, p->type, p, incr(curtok, p, consttree(1, inttype))), p);
            curtok = gettok();
            break;
        case '[': {
            Tree q;
            curtok = gettok();
            q = expr(']');
            if (YYnull) {
                if (isptr(p->type)) {
                    p = nullcheck(p);
                } else if (isptr(q->type)) {
                    q = nullcheck(q);
                }
            }
            p = (*optree['+'])(ADD, pointer(p), pointer(q));
            if (isptr(p->type) && isarray(p->type->type))
                p = retype(p, p->type->type);
            else
                p = rvalue(p);
        } break;
        case '(': {
            Type ty;
            Coordinate pt;
            p = pointer(p);
            if (isptr(p->type) && isfunc(p->type->type))
                ty = p->type->type;
            else {
                error("found `%t' expected a function\n", p->type);
                ty = func(voidtype, NULL, 1);
                p  = retype(p, ptr(ty));
            }
            pt = src;
            curtok  = gettok();
            p  = call(p, ty, pt);
        } break;
        case '.':
            curtok = gettok();
            if (curtok == ID) {
                if (isstruct(p->type)) {
                    Tree q = addrof(p);
                    p      = field(q, token);
                    q      = rightkid(q);
                    if (isaddrop(q->op) && q->u.sym->temporary)
                        p = tree(RIGHT, p->type, p, NULL);
                } else
                    error("left operand of . has incompatible type `%t'\n", p->type);
                curtok = gettok();
            } else
                error("field name expected\n");
            break;
        case DEREF:
            curtok = gettok();
            p = pointer(p);
            if (curtok == ID) {
                if (isptr(p->type) && isstruct(p->type->type)) {
                    if (YYnull)
                        p = nullcheck(p);
                    p = field(p, token);
                } else
                    error("left operand of -> has incompatible type `%t'\n", p->type);

                curtok = gettok();
            } else
                error("field name expected\n");
            break;
        default:
            return p;
        }
}

static Tree primary(void)
{
    Tree p;

    assert(curtok != '(');
    switch (curtok) {
    case ICON:
    case FCON:
        p      = tree(mkop(CNST, tsym->type), tsym->type, NULL, NULL);
        p->u.v = tsym->u.c.v;
        break;
    case SCON:
        if (ischar(tsym->type->type))
            tsym->u.c.v.p = stringn(tsym->u.c.v.p, tsym->type->size);
        else
            tsym->u.c.v.p =
                memcpy(allocate((tsym->type->size / widechar->size) * sizeof(int), PERM),
                       tsym->u.c.v.p, (tsym->type->size / widechar->size) * sizeof(int));
        tsym = constant(tsym->type, tsym->u.c.v);
        if (tsym->u.c.loc == NULL)
            tsym->u.c.loc = genident(STATIC, tsym->type, GLOBAL);
        p = idtree(tsym->u.c.loc);
        break;
    case ID:
        if (tsym == NULL) {
            Symbol s = install(token, &identifiers, level, PERM);
            s->src   = src;
            if (getchr() == '(') {
                Symbol q  = lookup(token, externals);
                s->type   = func(inttype, NULL, 1);
                s->sclass = EXTERN;
                if (Aflag >= 1)
                    warning("missing prototype\n");
                if (q && !eqtype(q->type, s->type, 1))
                    warning(
                        "implicit declaration of `%s' does not match previous declaration at %w\n",
                        q->name, &q->src);

                if (q == NULL) {
                    q         = install(s->name, &externals, GLOBAL, PERM);
                    q->type   = s->type;
                    q->sclass = EXTERN;
                    q->src    = src;
                    (*IR->defsymbol)(q);
                }
                s->u.alias = q;
            } else {
                error("undeclared identifier `%s'\n", s->name);
                s->sclass = AUTO;
                s->type   = inttype;
                if (s->scope == GLOBAL)
                    (*IR->defsymbol)(s);
                else
                    addlocal(s);
            }
            curtok = gettok();
            if (xref)
                use(s, src);
            return idtree(s);
        }
        if (xref)
            use(tsym, src);
        if (tsym->sclass == ENUM)
            p = consttree(tsym->u.value, inttype);
        else {
            if (tsym->sclass == TYPEDEF)
                error("illegal use of type name `%s'\n", tsym->name);
            p = idtree(tsym);
        }
        break;
    case FIRSTARG:
        if (level > PARAM && cfunc && cfunc->u.f.callee[0])
            p = idtree(cfunc->u.f.callee[0]);
        else {
            error("illegal use of `%k'\n", FIRSTARG);
            p = cnsttree(inttype, 0L);
        }
        break;
    default:
        error("illegal expression\n");
        p = cnsttree(inttype, 0L);
    }
    curtok = gettok();
    return p;
}

Tree idtree(Symbol p)
{
    int op;
    Tree e;
    Type ty = p->type ? unqual(p->type) : voidptype;

    if (p->scope == GLOBAL || p->sclass == STATIC)
        op = ADDRG;
    else if (p->scope == PARAM) {
        op = ADDRF;
        if (isstruct(p->type) && !IR->wants_argb) {
            e        = tree(mkop(op, voidptype), ptr(ptr(p->type)), NULL, NULL);
            e->u.sym = p;
            return rvalue(rvalue(e));
        }
    } else if (p->sclass == EXTERN) {
        assert(p->u.alias);
        p  = p->u.alias;
        op = ADDRG;
    } else
        op = ADDRL;
    p->ref += refinc;
    if (isarray(ty))
        e = tree(mkop(op, voidptype), p->type, NULL, NULL);
    else if (isfunc(ty))
        e = tree(mkop(op, funcptype), p->type, NULL, NULL);
    else
        e = tree(mkop(op, voidptype), ptr(p->type), NULL, NULL);
    e->u.sym = p;
    if (isptr(e->type))
        e = rvalue(e);
    return e;
}

Tree rvalue(Tree p)
{
    Type ty = deref(p->type);

    ty = unqual(ty);
    return tree(mkop(INDIR, ty), ty, p, NULL);
}

Tree lvalue(Tree p)
{
    if (generic(p->op) != INDIR) {
        error("lvalue required\n");
        return value(p);
    } else if (unqual(p->type) == voidtype)
        warning("`%t' used as an lvalue\n", p->type);
    return p->kids[0];
}

Tree retype(Tree p, Type ty)
{
    Tree q;

    if (p->type == ty)
        return p;
    q       = tree(p->op, ty, p->kids[0], p->kids[1]);
    q->node = p->node;
    q->u    = p->u;
    return q;
}

Tree rightkid(Tree p)
{
    while (p && p->op == RIGHT)
        if (p->kids[1])
            p = p->kids[1];
        else if (p->kids[0])
            p = p->kids[0];
        else
            unreachable();
    assert(p);
    return p;
}

int hascall(Tree p)
{
    if (p == 0)
        return 0;
    if (generic(p->op) == CALL ||
        (IR->mulops_calls && (p->op == DIV + I || p->op == MOD + I || p->op == MUL + I ||
                              p->op == DIV + U || p->op == MOD + U || p->op == MUL + U)))
        return 1;
    return hascall(p->kids[0]) || hascall(p->kids[1]);
}

Type binary(Type xty, Type yty)
{
#define xx(t) if (xty == t || yty == t) return t
    xx(longdouble);
    xx(doubletype);
    xx(floattype);
    xx(unsignedlonglong);
    xx(longlong);
    xx(unsignedlong);
    if ((xty == longtype && yty == unsignedtype) ||
        (xty == unsignedtype && yty == longtype)) {
        if (longtype->size > unsignedtype->size) {
            return longtype;
        } else {
            return unsignedlong;
        }
    }
    xx(longtype);
    xx(unsignedtype);
    return inttype;
#undef xx
}

Tree pointer(Tree p)
{
    if (isarray(p->type))
        /* assert(p->op != RIGHT || p->u.sym == NULL), */
        p = retype(p, atop(p->type));
    else if (isfunc(p->type))
        p = retype(p, ptr(p->type));
    return p;
}

Tree cond(Tree p)
{
    int op = generic(rightkid(p)->op);

    if (op == AND || op == OR || op == NOT || op == EQ || op == NE || op == LE || op == LT ||
        op == GE || op == GT)
        return p;
    p = pointer(p);
    return (*optree[NEQ])(NE, p, consttree(0, inttype));
}

Tree cast(Tree p, Type type)
{
    Type source, dest;

    p = value(p);
    if (p->type == type)
        return p;
    dest = unqual(type);
    source = unqual(p->type);
    if (source->op != dest->op || source->size != dest->size) {
        switch (source->op) {
        case INT:
            if (source->size < inttype->size)
                p = simplify(CVI, inttype, p, NULL);
            break;
        case UNSIGNED:
            if (source->size < inttype->size)
                p = simplify(CVU, inttype, p, NULL);
            else if (source->size < unsignedtype->size)
                p = simplify(CVU, unsignedtype, p, NULL);
            break;
        case ENUM:
            p = retype(p, inttype);
            break;
        case POINTER:
            if (isint(dest) && source->size > dest->size)
                warning("conversion from `%t' to `%t' is undefined\n", p->type, type);
            p = simplify(CVP, super(source), p, NULL);
            break;
        case FLOAT:
            break;
        default:
            unreachable();
        }
        {
            source = unqual(p->type);
            dest = super(dest);
            if (source->op != dest->op)
                switch (source->op) {
                case INT:
                    p = simplify(CVI, dest, p, NULL);
                    break;
                case UNSIGNED:
                    if (isfloat(dest)) {
                        Type ssrc = signedint(source);
                        Tree two  = cnsttree(longdouble, (/*long*/ double)2.0);
                        p         = (*optree['+'])(
                            ADD,
                            (*optree['*'])(
                                MUL, two,
                                simplify(CVU, ssrc, simplify(RSH, source, p, consttree(1, inttype)),
                                                 NULL)),
                            simplify(CVU, ssrc, simplify(BAND, source, p, consttree(1, unsignedtype)),
                                             NULL));
                    } else
                        p = simplify(CVU, dest, p, NULL);
                    break;
                case FLOAT:
                    if (isunsigned(dest)) {
                        Type sdst = signedint(dest);
                        Tree c =
                            cast(cnsttree(longdouble, (/*long*/ double)sdst->u.sym->u.limits.max.i + 1),
                                 source);
                        p = condtree(
                            simplify(GE, source, p, c),
                            (*optree['+'])(
                                ADD, cast(cast(simplify(SUB, source, p, c), sdst), dest),
                                cast(cnsttree(unsignedlong,
                                              (unsigned long)sdst->u.sym->u.limits.max.i + 1),
                                     dest)),
                            simplify(CVF, sdst, p, NULL));
                    } else
                        p = simplify(CVF, dest, p, NULL);
                    break;
                default:
                    unreachable();
                }
            dest = unqual(type);
        }
    }
    source = unqual(p->type);
    switch (source->op) {
    case INT:
        if (source->op != dest->op || source->size != dest->size)
            p = simplify(CVI, dest, p, NULL);
        break;
    case UNSIGNED:
        if (source->op != dest->op || source->size != dest->size)
            p = simplify(CVU, dest, p, NULL);
        break;
    case FLOAT:
        if (source->op != dest->op || source->size != dest->size)
            p = simplify(CVF, dest, p, NULL);
        break;
    case POINTER:
        if (source->op != dest->op)
            p = simplify(CVP, dest, p, NULL);
        else {
            if ((isfunc(source->type) && !isfunc(dest->type)) ||
                (!isfunc(source->type) && isfunc(dest->type)))
                warning("conversion from `%t' to `%t' is compiler dependent\n", p->type, type);

            if (source->size != dest->size)
                p = simplify(CVP, dest, p, NULL);
        }
        break;
    default:
        unreachable();
    }
    return retype(p, type);
}

Tree field(Tree p, const char *name)
{
    Field q;
    Type ty1, ty = p->type;

    if (isptr(ty))
        ty = deref(ty);
    ty1 = ty;
    ty  = unqual(ty);
    if ((q = fieldref(name, ty)) != NULL) {
        if (isarray(q->type)) {
            ty = q->type->type;
            if (isconst(ty1) && !isconst(ty))
                ty = qual(CONST, ty);
            if (isvolatile(ty1) && !isvolatile(ty))
                ty = qual(VOLATILE, ty);
            ty = array(ty, q->type->size / ty->size, q->type->align);
        } else {
            ty = q->type;
            if (isconst(ty1) && !isconst(ty))
                ty = qual(CONST, ty);
            if (isvolatile(ty1) && !isvolatile(ty))
                ty = qual(VOLATILE, ty);
            ty = ptr(ty);
        }
        if (YYcheck && !isaddrop(p->op) && q->offset > 0)                /* omit */
            p = nullcall(ty, YYcheck, p, consttree(q->offset, inttype)); /* omit */
        else                                                             /* omit */
            p = simplify(ADD + P, ty, p, consttree(q->offset, signedptr));

        if (q->lsb) {
            p          = tree(FIELD, ty->type, rvalue(p), NULL);
            p->u.field = q;
        } else if (!isarray(q->type))
            p = rvalue(p);

    } else {
        error("unknown field `%s' of `%t'\n", name, ty);
        p = rvalue(retype(p, ptr(inttype)));
    }
    return p;
}

/* funcname - return name of function f or a function' */
char *funcname(Tree f)
{
    if (isaddrop(f->op))
        return stringf("`%s'", f->u.sym->name);
    return "a function";
}

static Tree nullcheck(Tree p)
{
    if (!needconst && YYnull && isptr(p->type)) {
        p = value(p);
        if (strcmp(YYnull->name, "_YYnull") == 0) {
            Symbol t1 = temporary(REGISTER, voidptype);
            p         = tree(RIGHT, p->type,
                             tree(OR, voidtype, cond(asgn(t1, cast(p, voidptype))),
                                  vcall(YYnull, voidtype,
                                        (file && *file ? pointer(idtree(mkstr(file)->u.c.loc))
                                                       : cnsttree(voidptype, NULL)),
                                        cnsttree(inttype, (long)lineno), NULL)),
                             idtree(t1));
        }

        else
            p = nullcall(p->type, YYnull, p, cnsttree(inttype, 0L));
    }
    return p;
}

Tree nullcall(Type pty, Symbol f, Tree p, Tree e)
{
    Type ty;

    if (isarray(pty))
        return retype(nullcall(atop(pty), f, p, e), pty);
    ty = unqual(unqual(p->type)->type);
    return vcall(
        f, pty, p, e, cnsttree(inttype, (long)ty->size), cnsttree(inttype, (long)ty->align),
        (file && *file ? pointer(idtree(mkstr(file)->u.c.loc)) : cnsttree(voidptype, NULL)),
        cnsttree(inttype, (long)lineno), NULL);
}
