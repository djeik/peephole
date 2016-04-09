/*
 * JOOS is Copyright (C) 1997 Laurie Hendren & Michael I. Schwartzbach
 *
 * Reproduction of all or part of this software is permitted for
 * educational or research use on condition that this copyright notice is
 * included in any copy. This software comes with no warranty of any
 * kind. In no event will the authors be liable for any damages resulting from
 * use of this software.
 *
 * email: hendren@cs.mcgill.ca, mis@brics.dk
 */

 #include <stdlib.h>

int MAX_LOCALS = 0;
CODE *METHOD_START = NULL;

/*
 * Compare two CODEs.
 * A code C is equal to a code D if they have the same kind, and the value
 * associated with that kind in C is equal to the value associated with that
 * kind in D.
 */
int codecmp(CODE *c1, CODE *c2) {
    if(c1->kind != c2->kind)
        return 0;

    switch(c1->kind)
    {
        case newCK:
        case instanceofCK:
        case checkcastCK:
        case ldc_stringCK:
        case getfieldCK:
        case putfieldCK:
        case invokevirtualCK:
        case invokenonvirtualCK:
            return strcmp(c1->val.getfieldC, c2->val.getfieldC) == 0;
            break;
        case labelCK:
        case gotoCK:
        case ifeqCK:
        case ifneCK:
        case if_acmpeqCK:
        case if_acmpneCK:
        case ifnullCK:
        case ifnonnullCK:
        case if_icmpeqCK:
        case if_icmpgtCK:
        case if_icmpltCK:
        case if_icmpleCK:
        case if_icmpgeCK:
        case if_icmpneCK:
        case aloadCK:
        case astoreCK:
        case iloadCK:
        case istoreCK:
        case ldc_intCK:
            return c1->val.iloadC == c2->val.iloadC;
            break;
        case iincCK:
            return memcmp((void*)&c1->val, (void*)&c2->val, sizeof(c1->val.iincC));
            break;
        default:
            fprintf(stderr, "failed to compare CODE\n");
            return 0;
    }
}

typedef CODE *(*makeCODElabelF)(int, CODE *);

makeCODElabelF get_if(CODE *c, int *label)
{
    if(is_ifeq(c, label)) {
        return makeCODEifeq;
    }
    if(is_ifne(c, label)) {
        return makeCODEifne;
    }
    if(is_if_acmpeq(c, label)) {
        return makeCODEif_acmpeq;
    }
    if(is_if_acmpne(c, label)) {
        return makeCODEif_acmpne;
    }
    if(is_ifnull(c, label)) {
        return makeCODEifnull;
    }
    if(is_ifnonnull(c, label)) {
        return makeCODEifnonnull;
    }
    if(is_if_icmpeq(c, label)) {
        return makeCODEif_icmpeq;
    }
    if(is_if_icmpgt(c, label)) {
        return makeCODEif_icmpgt;
    }
    if(is_if_icmplt(c, label)) {
        return makeCODEif_icmplt;
    }
    if(is_if_icmple(c, label)) {
        return makeCODEif_icmple;
    }
    if(is_if_icmpge(c, label)) {
        return makeCODEif_icmpge;
    }
    if(is_if_icmpne(c, label)) {
        return makeCODEif_icmpne;
    }

    return NULL;
}

/* Same as get_if, but returns a function for the negation of the if. */
makeCODElabelF get_if_neg(CODE *c, int *label)
{
    if(is_ifeq(c, label)) {
        return makeCODEifne;
    }
    if(is_ifne(c, label)) {
        return makeCODEifeq;
    }
    if(is_if_acmpeq(c, label)) {
        return makeCODEif_acmpne;
    }
    if(is_if_acmpne(c, label)) {
        return makeCODEif_acmpeq;
    }
    if(is_ifnull(c, label)) {
        return makeCODEifnonnull;
    }
    if(is_ifnonnull(c, label)) {
        return makeCODEifnull;
    }
    if(is_if_icmpeq(c, label)) {
        return makeCODEif_icmpne;
    }
    if(is_if_icmpgt(c, label)) {
        return makeCODEif_icmple;
    }
    if(is_if_icmplt(c, label)) {
        return makeCODEif_icmpge;
    }
    if(is_if_icmple(c, label)) {
        return makeCODEif_icmpgt;
    }
    if(is_if_icmpge(c, label)) {
        return makeCODEif_icmplt;
    }
    if(is_if_icmpne(c, label)) {
        return makeCODEif_icmpeq;
    }

    return NULL;
}

/* astore_n
 * aload_n
 * -------->
 *  nothing
 *
 * aload_n
 * astore_n
 * -------->
 *  nothing
 */
int simplify_astore_aload(CODE **c)
{
    int n1, n2;
    if (
             is_astore(*c, &n1) &&
             is_aload(next(*c), &n2) &&
             n1 == n2
    ) {
        return
            replace(
                c,
                2,
                makeCODEdup(
                    makeCODEastore(
                        n1,
                        NULL
                    )
                )
        );
    }

    if (
            (
             is_aload(*c, &n1) &&
             is_astore(next(*c), &n2) &&
             n1 == n2
            )
    ) {
        return replace(c, 2, NULL);
    }

    return 0;
}

/* istore_n
 * iload_n
 * -------->
 *  nothing
 *
 * iload_n
 * istore_n
 * -------->
 *  nothing
 */
int simplify_istore_iload(CODE **c)
{
    int n1, n2;
    if (
             is_istore(*c, &n1) &&
             is_iload(next(*c), &n2) &&
             n1 == n2
    ) {
        return
            replace(
                c,
                2,
                makeCODEdup(
                    makeCODEistore(
                        n1,
                        NULL
                    )
                )
        );
    }

    if (
            (
             is_iload(*c, &n1) &&
             is_istore(next(*c), &n2) &&
             n1 == n2
            )
    ) {
        return replace(c, 2, NULL);
    }

    return 0;
}

/*
 * dup
 * istore_n
 * pop
 * -------->
 *  istore_n
 */
int simplify_istore(CODE **c)
{
    int x;
    if (
            is_dup(*c) &&
            is_istore(next(*c), &x) &&
            is_pop(next(next(*c)))
    ) {
        replace(c, 3, makeCODEistore(x, NULL));
    }

    return 0;
}

/*
 * nop
 * --->
 *  nothing
 */
int remove_nop(CODE **c)
{
    if (is_nop(*c))
    {
        return replace(c, 1, NULL);
    }

    return 0;
}

/* iload x        iload x        iload x
 * ldc 0          ldc 1          ldc 2
 * imul           imul           imul
 * ------>        ------>        ------>
 * ldc 0          iload x        iload x
 *                               dup
 *                               iadd
 */

int simplify_multiplication_right(CODE **c)
{
    int x, k;
    if (
          is_iload(*c,&x) &&
          is_ldc_int(next(*c),&k) &&
          is_imul(next(next(*c)))
    ) {
        if (k==0)
            return replace(c,3,makeCODEldc_int(0,NULL));
        else if (k==1)
            return replace(c,3,makeCODEiload(x,NULL));
        else if (k==2)
            return replace(
                    c,
                    3,
                    makeCODEiload(
                        x,
                        makeCODEdup(makeCODEiadd(NULL))
                    )
            );
        return 0;
    }
    return 0;
}

/* dup
 * astore x
 * pop
 * -------->
 * astore x
 */
int simplify_astore(CODE **c)
{
    int x;
    if (
            is_dup(*c) &&
            is_astore(next(*c),&x) &&
            is_pop(next(next(*c)))
    ) {
        return replace(c,3,makeCODEastore(x,NULL));
    }
    return 0;
}

/* iload x
 * ldc k   (0<=k<=127)
 * iadd
 * istore x
 * --------->
 * iinc x k
 */
int positive_increment(CODE **c)
{
    int x,y,k;
    if (
            is_iload(*c,&x) &&
            is_ldc_int(next(*c),&k) &&
            is_iadd(next(next(*c))) &&
            is_istore(next(next(next(*c))),&y) &&
            x==y && 0<=k && k<=127
    ) {
        return replace(c,4,makeCODEiinc(x,k,NULL));
    }
    return 0;
}

/* goto L1
 * ...
 * L1:
 * goto L2
 * ...
 * L2:
 * --------->
 * goto L2
 * ...
 * L1:    (reference count reduced by 1)
 * goto L2
 * ...
 * L2:    (reference count increased by 1)
 */
int simplify_goto_goto(CODE **c)
{
    int l1,l2;
    if (
            is_goto(*c,&l1) &&
            is_goto(next(destination(l1)),&l2) &&
            l1>l2
    ) {
        droplabel(l1);
        copylabel(l2);
        return replace(c,1,makeCODEgoto(l2,NULL));
    }
    return 0;
}

/*
 * iconst_0
 * goto L
 * ...
 * L:
 * ifeq M
 * -------->
 *  goto M
 */
int const_goto_ifeq(CODE **c)
{
    int n, l, d;
    if (
            is_ldc_int(*c, &n) &&
            !n &&
            is_goto(next(*c), &l) &&
            is_ifeq(next(destination(l)), &d)
    ) {
        droplabel(l);
        copylabel(d);
        return
            replace(
                    c,
                    2,
                    makeCODEgoto(
                        d,
                        NULL
                    )
            );
    }

    return 0;
}

/*
null can be cast to anything, so there's no need to check explicitly.
    aconst_null
    checkcast x
    ------->
    aconst_null
*/
int remove_checkcast_on_null(CODE **c)
{
    char* name;
    if (
        is_aconst_null(*c) &&
        is_checkcast(next(*c), &name)
    ) {
        return replace(c,2,makeCODEaconst_null(NULL));
    }
    return 0;
}


/*
A more sophisticated version of dup ... pop removal, specifically for putfields.
    dup
    aload_0
    swap
    putfield x
    pop
    ------>
    aload_0
    swap
    putfield x
*/
int simplify_putfield(CODE **c)
{
    int n;
    char *f;
    if (
        is_dup(*c) &&
        is_aload(next(*c), &n) &&
        !n &&
        is_swap(next(next(*c))) &&
        is_putfield(next(next(next(*c))), &f) &&
        is_pop(next(next(next(next(*c)))))
    ) {
        return replace(c, 5,
            makeCODEaload(n,
                makeCODEswap(
                    makeCODEputfield(f, NULL))));
    }

    return 0;
}


/*
A check is generated to replace nulls bu "null" when calling println. This
occurs even when we're calling with a string literal, which is of course
unnecessary.

    ldc "a string constant"
    dup
    ifnull null_x
    goto stop_x
    null_x:
    pop
    ldc "null"
    stop_x:
    ------>
    ldc "a string constant"

Of course, those labels should be unique.

This may be an oddly specific optmization, but it saves 7 (SEVEN!!) instructions.
*/
int remove_nullcheck_const_str(CODE **c) {
    char *s, *s2;
    int lbl_null, lbl_stop, la, lb;
    if (
        is_ldc_string(*c, &s) &&
        is_dup(next(*c)) &&
        is_ifnull(nextby(*c, 2), &lbl_null) &&
        is_goto(nextby(*c, 3), &lbl_stop) &&
        is_label(nextby(*c, 4), &la) &&
        la == lbl_null &&
        uniquelabel(la) &&
        is_pop(nextby(*c, 5)) &&
        is_ldc_string(nextby(*c, 6), &s2) &&
        strcmp(s2, "null") == 0 &&
        is_label(nextby(*c, 7), &lb) &&
        lb == lbl_stop &&
        uniquelabel(lb)
    ) {
        return replace_modified(c, 8, makeCODEldc_string(s, NULL));
    }

    return 0;
}

/*
After a return/goto, any instructions until a label can be safely removed.
(A good compiler shouldn't generate that kind of stuff outright but our
optimizations might cause it).
    [/i/a]return
    ...
    label:
    ---->
    [/i/a]return
    label:
*/
int remove_after_return_goto(CODE **c) {
    int n_after = 0;
    int l;
    int lgoto;
    CODE *curc, *ret;
    if (is_ireturn(*c) || is_areturn(*c) || is_return(*c) || is_goto(*c, &lgoto)) {
        curc = next(*c);

        /* Keep going until reaching a label, or the end of the code. */
        while(curc) {
            if (is_label(curc, &l))
                break;
            n_after++;
            curc = curc->next;
        }

        if (n_after == 0)
            return 0;

        if (is_ireturn(*c)) ret = makeCODEireturn(NULL);
        else if (is_areturn(*c)) ret = makeCODEareturn(NULL);
        else if (is_return(*c)) ret = makeCODEreturn(NULL);
        else {
            ret = makeCODEgoto(lgoto, NULL);
            copylabel(lgoto);
        }

        return replace_modified(c, n_after + 1, ret);
    }

    return 0;
}

/*
    Removes any dead labels.
    This is not really an "optimization" by itself since it doesn't change the
    size of the code, but removing dead labels may allow other patterns to
    match.
*/
int remove_dead_labels(CODE **c) {
    int n;
    if (is_label(*c, &n) && deadlabel(n)) {
        return replace_modified(c, 1, NULL);
    }

    return 0;
}

/* goto L
 * ...
 * L:
 * return
 * ------->
 *  return (refcount of L--)
 */
int goto_return(CODE **c) {
    int l;

    if (
            is_goto(*c, &l) &&
            is_return(next(destination(l)))
    ) {
        droplabel(l);
        return replace(
            c,
            1,
            makeCODEreturn(NULL)
        );
    }

    return 0;
}

/*
    iconst [not 0]              iconst_0
    ifeq l                      ifeq l
    ---->                       ---->
    [nothing] (drop on l)       goto l
*/
int remove_iconst_ifeq(CODE **c) {
    int n;
    int l;
    if (is_ldc_int(*c, &n) && is_ifeq(next(*c), &l)) {
        if (n == 0) {
            return replace(c, 2, makeCODEgoto(l, NULL));
        } else {
            droplabel(l);
            return replace(c, 2, NULL);
        }
    }

    return 0;
}

/*
    aconst_null         aconst_null
    if_acmpeq lbl       if_acmpne lbl
    ----->              ----->
    ifnull lbl          ifnonnull
*/
int remove_aconst_null_in_cmp(CODE **c) {
    int l;

    if (
        is_aconst_null(*c) &&
        is_if_acmpeq(next(*c), &l)
    ) {
        return replace(c, 2, makeCODEifnull(l, NULL));
    }

    if (
        is_aconst_null(*c) &&
        is_if_acmpne(next(*c), &l)
    ) {
        return replace(c, 2, makeCODEifnonnull(l, NULL));
    }


    return 0;
}

/*
Flips conditions (integers and refs) in trivial goto cases.
    if_[cond] lbl1
    goto lbl2
    lbl1:
    --->
    if_[!cond] lbl2
    lbl1: (dropped)
*/
int flip_cond(CODE **c) {
    int l1, l2, lt;

    if (
        is_if(c, &l1) &&
        is_goto(next(*c), &l2) &&
        is_label(next(next(*c)), &lt) &&
        l1 == lt
    ) {
        droplabel(l1);
        return replace(c, 2, (get_if_neg(*c, &l1))(l2, NULL));
    }

    return 0;
}

/*
    ldc a //string
    ldc b //string
    invokevirtual java/lang/String/concat
    ---->
    ldc a+b
*/
int collapse_ldc_string(CODE **c) {
    char *s1, *s2, *method, *result;
    char *STR_CONCAT = "java/lang/String/concat";
    int n;

    if (
        is_ldc_string(*c, &s1) &&
        is_ldc_string(next(*c), &s2) &&
        is_invokevirtual(next(next(*c)), &method) &&
        strncmp(method, STR_CONCAT, strlen(STR_CONCAT)) == 0
    ) {
        n = strlen(s1);
        result = malloc(n + strlen(s2) + 1);
        strcpy(result, s1);
        strcpy(result + n, s2);

        return replace(c, 3, makeCODEldc_string(result, NULL));
    }

    return 0;
}

/*
    new x
    dup
    invokenonvirtual x/<init>
    aload_0
    swap
    putfield
    ---->
    aload_0
    new x
    dup
    invokenonvirtual x/<init>
    putfield

    In reality, what we REALLY want is something that goes
    "if we have aload 0/swap/putfield, remove the swap and put the aload 0
    just before when that other value on the stack was created", but we can never
    look behind us, as we are condemned to stare at A+'s ever-advancing back.
*/
int remove_swaps_in_field_init(CODE **c) {
    char *niew, *init, *field;
    char *target_init;
    int n, len_new, r;
    if (
        is_new(*c, &niew) &&
        is_dup(next(*c)) &&
        is_invokenonvirtual(nextby(*c, 2), &init) &&
        is_aload(nextby(*c, 3), &n) &&
        n == 0 &&
        is_swap(nextby(*c, 4)) &&
        is_putfield(nextby(*c, 5), &field)
    ) {
        len_new = strlen(niew);
        target_init = malloc(len_new + 8);
        strcpy(target_init, niew);
        strcpy(target_init + len_new, "/<init>");
        r = strncmp(target_init, init, len_new + 7);
        free(target_init);

        if (r == 0) {
            return replace(c, 6,
                makeCODEaload(0,
                makeCODEnew(niew,
                makeCODEdup(
                makeCODEinvokenonvirtual(init,
                makeCODEputfield(field,
                NULL))))));
        }
    }

    return 0;
}

/*
    ldc a
    ldc a
    --->
    ldc a
    dup
*/
int dup_duplicate_consts(CODE **c) {
    char *s1, *s2;
    int a, b;

    if (
        is_ldc_int(*c, &a) &&
        is_ldc_int(next(*c), &b) &&
        a == b
    ) {
        return replace(c, 2, makeCODEldc_int(a, makeCODEdup(NULL)));
    }

    if (
        is_ldc_string(*c, &s1) &&
        is_ldc_string(next(*c), &s2) &&
        strcmp(s1, s2) == 0
    ) {
        return replace(c, 2, makeCODEldc_string(s1, makeCODEdup(NULL)));
    }


    return 0;
}

int nothing(CODE **c) {
    return 0;
}

/* if_icmplt L
 * goto M
 * L: (L has only one reference)
 * iconst_1
 * N: (N is dead)
 * ifeq O
 * x
 * ------------>
 *  if_icmplt L
 *  goto M
 *  L:
 *  x
 */
int if_iconst_ifeq(CODE **c)
{
    int l1, l2, m, n, o, i;

    int isreturn = 0;

    makeCODElabelF maker = NULL;

    if (
            (maker = get_if(*c, &l1)) &&         /* if_icmplt L */
            (
             is_goto(next(*c), &m)
             ||
             (isreturn = is_return(next(*c)))
            ) &&         /* goto M / return */
            is_label(next(next(*c)), &l2) && /* L: */ l1 == l2 &&
            uniquelabel(l1) &&
            is_ldc_int(next(next(next(*c))), &i) && /* iconst_1 */
            is_label(nextby(*c, 4), &n) && /* N: */
            deadlabel(n) &&
            is_ifeq(nextby(*c, 5), &o) /* ifeq O */
    ) {
        CODE *l = makeCODElabel(l1, NULL);

        return
            replace(c,
                    6,
                    maker(
                        l1,
                        isreturn ?
                        makeCODEreturn(
                            l
                        )
                        :
                        makeCODEgoto(
                            m,
                            l
                        )
                    )
            );
    }

    return 0;
}

int load_and_swap(CODE **c)
{
    int n1;
    char *s1;
    CODE *c1, *c2;

    if (is_ldc_int(*c, &n1)) {
        c2 = makeCODEldc_int(n1, NULL);
    }
    else if (is_ldc_string(*c, &s1)) {
        c2 = makeCODEldc_string(s1, NULL);
    }
    else if (is_iload(*c, &n1)) {
        c2 = makeCODEiload(n1, NULL);
    }
    else if (is_aload(*c, &n1)) {
        c2 = makeCODEaload(n1, NULL);
    }
    else {
        return 0;
    }

    if (is_ldc_int(next(*c), &n1)) {
        c1 = makeCODEldc_int(n1, c2);
    }
    else if (is_ldc_string(next(*c), &s1)) {
        c1 = makeCODEldc_string(s1, c2);
    }
    else if (is_iload(next(*c), &n1)) {
        c1 = makeCODEiload(n1, c2);
    }
    else if (is_aload(next(*c), &n1)) {
        c1 = makeCODEaload(n1, c2);
    }
    else {
        return 0;
    }

    if (is_swap(nextby(*c, 2))) {
        return replace(c, 3, c1);
    }

    return 0;
}

/* aload n
 * swap
 * putfield f
 * aload n
 * getfield f
 * ---------->
 *  dup
 *  aload n
 *  swap
 *  putfield f
 */
int put_and_get(CODE **c)
{
    int a1, a2;
    char *field1, *field2;
    if (
            is_aload(*c, &a1) &&
            is_swap(next(*c)) &&
            is_putfield(nextby(*c, 2), &field1) &&
            is_aload(nextby(*c, 3), &a2) &&
            is_getfield(nextby(*c, 4), &field2) &&
            a1 == a2 &&
            strcmp(field1, field2) == 0
    ) {
        return
            replace(
                c,
                5,
                makeCODEdup(
                    makeCODEaload(
                        a1,
                        makeCODEswap(
                            makeCODEputfield(
                                field1,
                                NULL
                            )
                        )
                    )
                )
            );
    }

    return 0;
}

/* if* L
 * c
 * c'
 * ...
 * L:
 * c
 * c'
 * ------> Provided L is unique
 *  c
 *  if* L
 *  c'
 *  ...
 *  L:
 *  c'
 */
int refactor_branch(CODE **c)
{
    makeCODElabelF maker;
    int l;

    if (
            (maker = get_if(*c, &l)) &&
            codecmp(next(*c), next(destination(l)))
    ) {
        CODE *common1, *common2;

        common1 = next(*c);
        common2 = next(destination(l));

        common1->next = maker(l, next(next(*c)));
        destination(l)->next = next(next(destination(l)));

        *c = common1;

        return 1;
    }

    return 0;
}

int is_load(CODE *d, int *n)
{
    return is_iload(d, n) || is_istore(d, n) || is_aload(d, n) || is_astore(d, n);
}

struct codelist
{
    CODE *here;
    codelist *there;
};

codelist *build_backlist(CODE *to_here, CODE *here, codelist *acc)
{
    if(here == NULL)
    {
        fprintf(stderr, "backlist construction failed\n");
        return NULL;
    }

    codelist *singleton = malloc(sizeof(*singleton));
    singleton->there = acc;
    singleton->here = here;

    if(here == to_here)
        return singleton;
    else
        return build_backlist(to_here, here->next, singleton);
}

int bookkeeping(CODE **c)
{
    static NEW_METHOD = 1;
    CODE *d = *c;
    METHOD_START = *c;

    if(*c == NULL || (*c)->next == NULL)
    {
        NEW_METHOD = 1;
        return 0;
    }

    if(!NEW_METHOD)
        return 0;

    MAX_LOCALS = 0;

    fprintf(stderr, "WHY?!\n");

    while(d != NULL)
    {
        int n;
        if(is_load(d, &n) && n > MAX_LOCALS)
            MAX_LOCALS = n;
        d = d->next;
    }

    fprintf(stderr, "REPLACE\n");
    replace(c, 0, makeCODEldc_int(0, makeCODEistore(MAX_LOCALS + 1, NULL)));

    NEW_METHOD = 0;

    fprintf(stderr, "BYE\n");

    return 0;
}

#define OPTS 100

OPTI optimization[] = {
    bookkeeping,
    const_goto_ifeq,
    if_iconst_ifeq,
    simplify_multiplication_right,
    simplify_astore,
    simplify_istore,
    positive_increment,
    simplify_goto_goto,
    simplify_astore_aload,
    simplify_istore_iload,
    remove_nop,
    remove_checkcast_on_null,
    simplify_putfield,
    goto_return,
    remove_nullcheck_const_str,
    remove_after_return_goto,
    remove_dead_labels,
    load_and_swap,
    remove_iconst_ifeq,
    flip_cond,
    collapse_ldc_string,
    remove_aconst_null_in_cmp,
    put_and_get,
    dup_duplicate_consts,
    remove_swaps_in_field_init,
    // refactor_branch, // broken
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing,
    nothing
};
