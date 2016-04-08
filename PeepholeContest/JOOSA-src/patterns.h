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
After a return, any instructions until a label can be safely removed.
(A good compiler shouldn't generate that kind of stuff outright but our
optimizations might cause it).
    [/i/a]return
    ...
    label:
    ---->
    [/i/a]return
    label:
*/
int remove_after_return(CODE **c) {
    int n_after = 0;
    int l;
    CODE *curc, *ret;
    if (is_ireturn(*c) || is_areturn(*c) || is_return(*c)) {
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
        else ret = makeCODEreturn(NULL);

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
Flips equality conditions in trivial goto cases.

    if_acmpeq lbl1                  if_acmpne lbl1
    goto lbl2                       goto lbl2
    lbl1:                           lbl1:
    --->                            --->
    if_acmpne lbl2 (drop lbl1)      ifacmpeq lbl2 (drop lbl1)
    lbl1:                           lbl1:
*/
int flip_eq(CODE **c) {
    int l1, l2, lt, flg = 0;
    if (is_if_acmpeq(*c, &l1))
        flg = 1;
    else if (is_if_acmpne(*c, &l1))
        flg = 2;

    if (
        flg &&
        is_goto(next(*c), &l2) &&
        is_label(next(next(*c)), &lt) &&
        l1 == lt
    ) {
        droplabel(l1);
        if (flg == 1) {
            return replace(c, 2, makeCODEif_acmpne(l2, NULL));
        } else {
            return replace(c, 2, makeCODEif_acmpeq(l2, NULL));
        }
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
int if_icmplt_iconst_ifeq(CODE **c)
{
    int l1, l2, m, n, o, i;

    int isreturn = 0;

    if (
            is_if_icmplt(*c, &l1) &&         /* if_icmplt L */
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
                    makeCODEif_icmplt(
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
    int n;
    char* s;
    int a;
    int i;

    int isaload;
    int isstring;

    if (
            (
             (isstring =
              (is_ldc_string(*c, &s) ? 1 : 0) || (is_ldc_int(*c, &n) ? -1 : 0)
             )
            )
            &&
            (
             (isaload =
              (is_aload(next(*c), &a) ? 1 : 0) || (is_iload(next(*c), &i) ? -1 : 0)
             )
            )
            &&
            is_swap(next(next(*c)))
    ) {
        CODE *stringcode = isstring == 1 ?
            makeCODEldc_string(
                s,
                NULL
            )
            :
            makeCODEldc_int(
                n,
                NULL
            );

        return
            replace(
                    c,
                    3,
                    isaload == 1 ?
                    makeCODEaload(
                        a,
                        stringcode
                    )
                    :
                    makeCODEiload(
                        i,
                        stringcode
                    )
            );
    }

    if (
            (
             (isaload =
              (is_aload(next(*c), &a) ? 1 : 0) || (is_iload(next(*c), &i) ? -1 : 0)
             )
            )
            &&
            (
             (isstring =
              (is_ldc_string(*c, &s) ? 1 : 0) || (is_ldc_int(*c, &n) ? -1 : 0)
             )
            )
            &&
            is_swap(next(next(*c)))
    ) {
        CODE *loadcode = isaload == 1 ?
            makeCODEaload(
                    a,
                    NULL
            )
            :
            makeCODEiload(
                    i,
                    NULL
            );

        return replace(
                c,
                3,
                isstring == 1 ?
                makeCODEldc_string(
                    s,
                    loadcode
                )
                :
                makeCODEldc_int(
                    n,
                    loadcode
                )
        );
    }

    return 0;
}

#define OPTS 100

OPTI optimization[] = {
    const_goto_ifeq,
    if_icmplt_iconst_ifeq,
    simplify_multiplication_right,
    simplify_astore,
    simplify_istore,
    positive_increment,
    simplify_goto_goto,
    simplify_astore_aload,
    remove_nop,
    remove_checkcast_on_null,
    simplify_putfield,
    goto_return,
    remove_nullcheck_const_str,
    remove_after_return,
    remove_dead_labels,
    load_and_swap,
    remove_iconst_ifeq,
    flip_eq,
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
    nothing,
    nothing,
    nothing,
    nothing,
    nothing
};
