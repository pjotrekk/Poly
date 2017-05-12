#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef long poly_coeff_t;
typedef int poly_exp_t;

enum UnionTest {
    SIMPLE,
    COMPLEX,
    ZERO
};

struct Poly;
typedef struct Poly Poly;

struct Mono;
typedef struct Mono Mono;

struct Poly {
    enum UnionTest tag;
    union {
        Mono *m;
        poly_coeff_t c;
    } type;
};

struct Mono {
    Poly p;
    poly_exp_t exp;
    struct Mono *next;
};


/**
 * Tworzy wielomian, który jest współczynnikiem.
 * @param[in] c : wartość współczynnika
 * @return wielomian
 */
static inline Poly PolyFromCoeff(poly_coeff_t c)
{
    printf("PolyFromCoeff\n");
    Poly p;
    p.tag = SIMPLE;
    p.type.c = c;
    return p;
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian
 */
static inline Poly PolyZero()
{
    printf("PolyZero\n");
    Poly p;
    p.tag = ZERO;
    p.type.m = NULL;
    return p;
}

/**
 * Tworzy jednomian `p * x^e`.
 * Przejmuje na własność zawartość struktury wskazywanej przez @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(const Poly *p, poly_exp_t e)
{
    printf("MonoFromPoly\n");
    Mono m;
    m.exp = e;
    m.p = *p;
    m.next = NULL;
    return m;
}

/**
 * Sprawdza, czy wielomian jest współczynnikiem.
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p)
{
    printf("PolyIsCoeff\n");
    return p->tag == SIMPLE;
}

/**
 * Sprawdza, czy wielomian jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return Czy wielomian jest równy zero?
 */
static inline bool PolyIsZero(const Poly *p)
{
    printf("PolyIsZero\n");
    return ((p->tag == COMPLEX) && (p->type.m == NULL))
            || ((p->tag == SIMPLE) && (p->type.c == 0))
            || (p->tag == ZERO);
}

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p) {
    printf("PolyDestroy\n");
    if (!PolyIsZero(p) && !PolyIsCoeff(p)) {
        while (p->type.m != NULL) {
            Mono *tmp = p->type.m;
            p->type.m = p->type.m->next;
            PolyDestroy(&tmp->p);
            free(tmp);
        }
    }
}

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m)
{
    printf("MonoDestroy\n");            // czy mogę założyć, że to jednomian w wielomianie (alokowany dynamicznie)??
    PolyDestroy(&m->p);
}

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p)
{
    printf("PolyClone\n");
    if (PolyIsZero(p)) {
        return PolyZero();
    } else if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->type.c);
    } else {
        Poly p_clone;
        p_clone.tag = p->tag;
        Mono *tmp = p->type.m;
        Mono *start = (Mono*) malloc(sizeof(Mono));
        assert(start != NULL);
        start->exp = tmp->exp;
        start->next = NULL;
        Mono *wanderer = start;
        Mono *helper;
        tmp = tmp->next;
        while (tmp != NULL) {
            helper = (Mono*) malloc(sizeof(Mono));
            assert(helper != NULL);
            helper->next = NULL;
            helper->exp = tmp->exp;
            wanderer->next = helper;
            wanderer = helper;
            tmp = tmp->next;
        }
        wanderer = start;
        tmp = p->type.m;
        while (wanderer != NULL) {
            wanderer->p = PolyClone(&tmp->p);
            wanderer = wanderer->next;
            tmp = tmp->next;
        }
        p_clone.type.m = start;
        return p_clone;
    }
}

/**
 * Robi pełną, głęboką kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m)
{
    printf("MonoClone\n");
    Mono new_m;
    new_m.exp = m->exp;
    new_m.p = PolyClone(&m->p);
    return new_m;
}



/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q)
{
    printf("PolyAdd\n");
    Mono *zeroexp;
    Poly added, new;
    if (PolyIsZero(p)) {
        return PolyClone(q);
    } else if (PolyIsZero(q)) {
        return PolyClone(p);
    } else if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        if (p->type.c + p->type.c != 0) {
            return PolyFromCoeff(p->type.c + p->type.c);
        }
        else {
            return PolyZero();
        }
    } else if (PolyIsCoeff(p) || PolyIsCoeff(q)) {
        zeroexp = (Mono *) malloc(sizeof(Mono));
        assert(zeroexp != NULL);
        zeroexp->exp = 0;
        zeroexp->next = NULL;
        new.tag = COMPLEX;
        new.type.m = zeroexp;
        if (PolyIsCoeff(p)) {
            zeroexp->p = PolyFromCoeff(p->type.c);
            added = PolyAdd(&new, q);
        } else {
            zeroexp->p = PolyFromCoeff(q->type.c);
            added = PolyAdd(&new, p);
        }
        PolyDestroy(&new);
        return added;
    } else {
        added.tag = COMPLEX;
        Mono *doll = (Mono*) malloc(sizeof(Mono));
        assert(doll != NULL);
        Mono *wanderer = doll;
        Mono *new_mono;
        Mono *mono_p = p->type.m;
        Mono *mono_q = q->type.m;
        while (mono_p != NULL && mono_q != NULL) {
            if (mono_p->exp != mono_q->exp) {
                new_mono = (Mono*) malloc(sizeof(Mono));
                assert(new_mono != NULL);
                new_mono->next = NULL;
                if (mono_p->exp > mono_q->exp) {
                    new_mono->exp = mono_p->exp;
                    new_mono->p = PolyClone(&mono_p->p);
                    mono_p = mono_p->next;
                } else {  // mono_p->exp > mono_q->exp
                    new_mono->exp = mono_q->exp;
                    new_mono->p = PolyClone(&mono_q->p);
                    mono_q = mono_q->next;
                }
                wanderer->next = new_mono;
                wanderer = wanderer->next;
            } else { //p.exp == q.exp
                new_mono = (Mono*) malloc(sizeof(Mono));
                assert(new_mono != NULL);
                new_mono->p = PolyAdd(&mono_p->p, &mono_q->p);
                if (PolyIsZero(&new_mono->p)) {
                    PolyDestroy(&new_mono->p);
                    free(new_mono);
                } else {
                    new_mono->next = NULL;
                    new_mono->exp = mono_p->exp;
                    wanderer->next = new_mono;
                    wanderer = wanderer->next;
                }
                mono_p = mono_p->next;
                mono_q = mono_q->next;
            }
        }

        if (mono_p != NULL || mono_q != NULL) {
            while (mono_p != NULL) {
                new_mono = (Mono*) malloc(sizeof(Mono));
                assert(new_mono != NULL);
                new_mono->exp = mono_p->exp;
                new_mono->next = NULL;
                new_mono->p = PolyClone(&mono_p->p);
                mono_p = mono_p->next;
            }
            while (mono_q != NULL) {
                new_mono = (Mono*) malloc(sizeof(Mono));
                assert(new_mono != NULL);
                new_mono->exp = mono_q->exp;
                new_mono->next = NULL;
                new_mono->p = PolyClone(&mono_q->p);
                mono_q = mono_q->next;
            }
        }

        added.type.m = doll->next;
        wanderer = added.type.m;
        free(doll);

        Poly helper;
        if (wanderer == NULL) {
            added = PolyZero();
        } else  { /*
            while (wanderer->p.tag == COMPLEX &&
                   wanderer->exp == 0 &&
                   wanderer->next == NULL) {
                helper = PolyAt(&added, 1);
                PolyDestroy(&added);
                added = helper;
                wanderer = added.type.m;
            } */
            if (wanderer->p.tag == SIMPLE && wanderer->next == NULL) {
                helper = added;
                added = PolyFromCoeff(added.type.m->p.type.c);
                PolyDestroy(&helper);
            }
        }
        return added;
    }
}


poly_exp_t MonoExpComparator(const void *x1, const void *x2) {
    printf("MonoExpComparator\n");
    Mono *y1 = (Mono*) x1;
    Mono *y2 = (Mono*) x2;
    return y2->exp - y1->exp;
}


/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(unsigned count, const Mono monos[])
{
    printf("PolyAddMonos\n");
    Poly p;
    Poly helper, destroyer;
    Mono *tmp = (Mono*) monos;
    qsort(tmp, count, sizeof(Mono), MonoExpComparator);
    unsigned i = 0;
    Mono *doll = (Mono*) malloc(sizeof(Mono));
    assert(doll != NULL);
    doll->exp = -1;
    Mono *backer = NULL;
    Mono *wanderer = doll;
    Mono *newMono;
    while (i < count) {
        if (!PolyIsZero(&tmp[i].p)) {
            printf("1\n");
            if (wanderer->exp != tmp[i].exp) {

                printf("2\n");
                newMono = (Mono*) malloc(sizeof(Mono));
                assert(newMono != NULL);
                newMono->next = NULL;
                newMono->exp = tmp[i].exp;
                newMono->p = tmp[i].p;
                wanderer->next = newMono;
                backer = wanderer;
                wanderer = wanderer->next;
            }
            else {
                helper = PolyAdd(&wanderer->p, &tmp[i].p);
                if (PolyIsZero(&helper)) {
                    PolyDestroy(&wanderer->p);
                    free(wanderer);
                    wanderer = backer;
                    wanderer->next = NULL;
                } else {
                    destroyer = wanderer->p;
                    MonoDestroy(&tmp[i]);
                    PolyDestroy(&destroyer);
                    wanderer->p = helper;
                }
            }
        }
        i++;
    }
    wanderer = doll->next;
    p.type.m = wanderer;
    free(doll);
    if (wanderer == NULL) {
        p = PolyZero();
    } else  {/*
        while (wanderer->p.tag == COMPLEX &&
               wanderer->exp == 0 &&
               wanderer->next == NULL) {
            helper = PolyAt(&p, 1);
            PolyDestroy(&p);
            p = helper;
            wanderer = p.type.m;
        } */
        if (wanderer->p.tag == SIMPLE && wanderer->next == NULL) {
            helper = p;
            p = PolyFromCoeff(wanderer->p.type.c);
            PolyDestroy(&helper);
        }
    }
    return p;

}



/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 *
 * */
Poly PolyMul(const Poly *p, const Poly *q)
{
    printf("PolyMul\n");
    if (PolyIsZero(p) || PolyIsZero(q)) {
        return PolyZero();
    } else if (PolyIsCoeff(p) || PolyIsCoeff(q)) {
        if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
            return PolyFromCoeff(p->type.c * p->type.c);
        } else {
            Poly toMultiply;
            Poly score;
            Mono *zeroexp = (Mono*) malloc(sizeof(Mono));
            assert(zeroexp != NULL);
            zeroexp->exp = 0;
            zeroexp->next = NULL;
            toMultiply.tag = COMPLEX;
            toMultiply.type.m = zeroexp;
            if (PolyIsCoeff(p)) {
                zeroexp->p = PolyFromCoeff(p->type.c);
                score = PolyMul(&toMultiply, q);
            } else {
                zeroexp->p = PolyFromCoeff(q->type.c);
                score = PolyMul(&toMultiply, p);
            }
            PolyDestroy(&toMultiply);
            return score;
        }
    } else {
        poly_exp_t counter = 0;
        Mono *header_p = p->type.m;
        while (header_p != NULL) {
            counter++;
            header_p = header_p->next;
        }
        Mono *header_q = q->type.m;
        while (header_q != NULL) {
            counter++;
            header_q = header_q->next;
        }
        Mono *monos = (Mono*) malloc(sizeof(Mono) * counter);
        poly_exp_t i = 0;
        header_p = p->type.m;
        while (header_p != NULL) {
            header_q = q->type.m;
            while (header_q != NULL) {
                monos[i].exp = header_p->exp + header_q->exp;
                monos[i].next = NULL;
                monos[i].p = PolyMul(&header_p->p, &header_q->p);
                i++;
            }
            header_p = header_p->next;
        }
        Poly score2 = PolyAddMonos((unsigned)counter, monos);
        free(monos);
        return score2;
    }
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p)
{
    printf("PolyNeg\n");
    if (PolyIsZero(p)) {
        return PolyZero();
    } else if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->type.c * (-1));
    } else {
        Poly p_neg;
        p_neg.tag = p->tag;
        Mono *tmp = p->type.m;
        Mono *start = (Mono*) malloc(sizeof(Mono));
        assert(start != NULL);
        start->exp = tmp->exp;
        start->next = NULL;
        Mono *wanderer = start;
        Mono *helper;
        tmp = tmp->next;
        while (tmp != NULL) {
            helper = (Mono*) malloc(sizeof(Mono));
            assert(helper != NULL);
            helper->next = NULL;
            helper->exp = tmp->exp;
            wanderer->next = helper;
            wanderer = helper;
            tmp = tmp->next;
        }
        wanderer = start;
        tmp = p->type.m;
        while (wanderer != NULL) {
            wanderer->p = PolyClone(&tmp->p);
            wanderer = wanderer->next;
            tmp = tmp->next;
        }
        p_neg.type.m = start;
        return p_neg;
    }
}

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q)
{
    printf("PolySub\n");
    Poly q_neg = PolyNeg(q);
    Poly subbed = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return subbed;
}

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru).
 * Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{
    printf("PolyDegBy\n");
    if (PolyIsZero(p)) {
        return -1;
    }
    else if (var_idx == 0) {
        return p->type.m->exp;
    }
    else {
        poly_exp_t deg = -2;
        poly_exp_t y;
        Mono *helper = p->type.m;
        while (helper != NULL) {
            y = PolyDegBy(&helper->p, var_idx - 1);
            if (y > deg) {
                deg = y;
            }
            helper = helper->next;
        }
        return deg;
    }
}

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p)
{
    printf("PolyDeg\n");
    if (PolyIsZero(p)) {
        return -1;
    }
    else if (PolyIsCoeff(p)) {
        return 0;
    }
    else {
        return p->type.m->exp;
    }
}

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q)
{
    printf("PolyIsEq\n");
    if (PolyIsZero(p) || PolyIsZero(q)) {
        return PolyIsZero(p) && PolyIsZero(q);
    } else if (PolyIsCoeff(p) || PolyIsCoeff(q)) {
        if (!(PolyIsCoeff(p) && PolyIsCoeff(q))) {
            return false;
        } else {
            return p->type.c == q->type.c;
        }
    } else {
        bool isEq = true;
        Mono *tmp_p = p->type.m;
        Mono *tmp_q = q->type.m;
        while (tmp_p != NULL && tmp_q != NULL) {
            if (tmp_p->exp != tmp_q->exp) {
                return false;
            } else {
                isEq = PolyIsEq(&tmp_p->p, &tmp_q->p);
                if (isEq == false) {                          //warunek chyba ok?
                    return false;
                }
            }
            tmp_p = tmp_p->next;
            tmp_q = tmp_q->next;
        }
        if (!(tmp_q == NULL && tmp_p == NULL)) {
            return false;
        }
        return isEq;
    }
}

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianem
 * i zmniejszane są indeksy zmiennych w takim wielomianie o jeden.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p
 * @param[in] x
 * @return @f$p(x, x_0, x_1, \ldots)@f$
*/
Poly PolyAt(const Poly *p, poly_coeff_t x)
{
    printf("PolyAt\n");
    Poly m;
    return m;
}