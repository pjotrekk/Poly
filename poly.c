#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

struct MonoList;
typedef struct MonoList MonoList;

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
Poly PolyClone(const Poly *p) {
    printf("PolyClone\n");
    if (PolyIsZero(p)) {
        return PolyZero();
    } else if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->type.c);
    } else {
        Poly p_clone;
        p_clone.tag = p->tag;
        Poly tmp = *p;
        Mono *start = (Mono*) malloc(sizeof(Mono));
        start->exp = tmp.type.m->exp;
        start->next = NULL;
        Mono *wanderer = start;
        Mono *helper;
        tmp.type.m = tmp.type.m->next;
        while (tmp.type.m != NULL) {
            helper = (Mono*) malloc(sizeof(Mono));
            helper->next = NULL;
            helper->exp = tmp.type.m->exp;
            wanderer->next = helper;
            wanderer = helper;
            tmp.type.m = tmp.type.m->next;
        }
        wanderer = start;
        tmp = *p;
        while (wanderer != NULL) {
            wanderer->p = PolyClone(&tmp.type.m->p);
            wanderer = wanderer->next;
            tmp.type.m = tmp.type.m->next;
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
    Poly added;
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
    } else if (PolyIsCoeff(p)) {
        zeroexp = (Mono*) malloc(sizeof(Mono));
        zeroexp->p = PolyFromCoeff(p->type.c);
        zeroexp->exp = 0;
        zeroexp->next = NULL;
        Poly new;
        new.tag = COMPLEX;
        new.type.m = zeroexp;
        added = PolyAdd(&new, q);
        free(zeroexp);
        return added;
    } else if (PolyIsCoeff(q)) {
        zeroexp = (Mono*) malloc(sizeof(Mono));
        zeroexp->p = PolyFromCoeff(p->type.c);
        zeroexp->exp = 0;
        zeroexp->next = NULL;
        Poly new;
        new.tag = COMPLEX;
        new.type.m = zeroexp;
        added = PolyAdd(&new, q);
        free(zeroexp);
        return added;
    } else {
        added.tag = COMPLEX;
        Mono *doll = (Mono*) malloc(sizeof(Mono));
        Mono *wanderer = doll;
        Mono *deleter, *temp;
        Poly help1 = PolyClone(p);
        Poly help2 = PolyClone(q);
        while (help1.type.m != NULL && help2.type.m != NULL) {
            if (help1.type.m->exp < help2.type.m->exp) {
                wanderer->next = help1.type.m;
                help1.type.m = help1.type.m->next;
                wanderer = wanderer->next;
            } else if (help1.type.m->exp > help2.type.m->exp) {
                wanderer->next = help2.type.m;
                help1.type.m = help2.type.m->next;
                wanderer = wanderer->next;
            } else { //help1.exp == help2.exp
                temp = (Mono*) malloc(sizeof(Mono));
                temp->p = PolyAdd(&help1.type.m->p, &help2.type.m->p);
                if (!PolyIsZero(&temp->p)) {
                    temp->next = NULL;
                    temp->exp = help1.type.m->exp;
                    wanderer->next = temp;
                    wanderer = wanderer->next;
                } else {
                    PolyDestroy(&temp->p);
                    free(temp);
                }
                deleter = help1.type.m;
                help1.type.m = help1.type.m->next;
                PolyDestroy(&deleter->p);
                free(deleter);// możliwe że polydestroy, a potem free(deleter);
                deleter = help2.type.m;
                help2.type.m = help2.type.m->next;
                PolyDestroy(&deleter->p);                // to samo
                free(deleter);
            }
        }
        if (help1.type.m != NULL) {
            wanderer->next = help1.type.m;
        } else if (help2.type.m != NULL) {
            wanderer->next = help2.type.m;
        }


        added.type.m = doll->next;
        free(doll);
        Poly helper;
        while (added.type.m->p.tag == COMPLEX && added.type.m->exp == 0 &&
                added.type.m->next == NULL) {                            // WTF, chyba PolyAt
      //      helper = PolyAt(&added, 1);
            PolyDestroy(&added);
            added = helper;
        }

        if (added.type.m == NULL) {
            added = PolyZero();
        } else if (added.type.m->p.tag == SIMPLE) {
            helper = added;
            added = PolyFromCoeff(added.type.m->p.type.c);
            PolyDestroy(&helper);
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
    Mono *tmp = (Mono*) monos;
    qsort(tmp, count, sizeof(Mono), MonoExpComparator);
    unsigned i = 0;
    Mono *doll = (Mono*) malloc(sizeof(Mono));
    doll->exp = -1;
    Mono *wanderer = doll;
    Mono *newMono;
    while (i < count) {
        if (!PolyIsZero(&tmp[i].p)) {
            printf("TU\n");
            newMono = (Mono*) malloc(sizeof(Mono));
            newMono->next = NULL;
            if (wanderer->exp != tmp[i].exp) {
                newMono->exp = tmp[i].exp;
                newMono->p = tmp[i].p;
                wanderer->next = newMono;
            }
            else {
                newMono->p = PolyAdd(&wanderer->p, &tmp[i].p);
                MonoDestroy(&tmp[i]);
                PolyDestroy(&wanderer->p);
                wanderer->p = newMono->p;
            }
        }
        i++;
    }
    wanderer = doll->next;
    free(doll);
    if (wanderer == NULL) {
        p = PolyZero();
    } else if (wanderer->exp == 0 && wanderer->next == NULL) {
        p = PolyFromCoeff(wanderer->p.type.c);
        free(wanderer);
    } else {
        p.tag = COMPLEX;
        p.type.m = wanderer;
    }
    return p;

}



/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q)
{
    printf("PolyMul\n");
    Poly m;
    return m;
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p)
{
    printf("PolyNeg\n");
    Poly m;
    return m;
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
    Poly m;
    return m;
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
    return -2;
}

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p)
{
    printf("PolyDeg\n");
    return -2;
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
    return false;
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