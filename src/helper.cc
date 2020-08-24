#include "helper.h"

#include <iostream>

void resolve(vec<Lit>& main, vec<Lit>& other, Var x)
{
    Lit     p;
    bool    ok1 = false, ok2 = false;
    for (int i = 0; i < main.size(); i++){
        if (var(main[i]) == x){
            ok1 = true, p = main[i];
            main[i] = main.last();
            main.pop();
            break;
        }
    }

    for (int i = 0; i < other.size(); i++){
        if (var(other[i]) != x)
            main.push(other[i]);
        else{
            if (p != ~other[i])
                printf("PROOF ERROR! Resolved on variable with SAME polarity in both clauses: %d\n", x+1);
            ok2 = true;
        }
    }

    if (!ok1 || !ok2)
        printf("PROOF ERROR! Resolved on missing variable: %d\n", x+1);

    sortUnique(main);
}

void Checker::root(const vec<Lit>& c) {
    // printf("%d: ROOT", clauses.size()); for (int i = 0; i < c.size(); i++) printf(" %s%d", sign(c[i])?"-":"", var(c[i])+1); printf("\n");
    std::cout << clauses.size() << ": ROOT";
    for (int i = 0; i < c.size(); i++) {
        std::cout << " " << (sign(c[i])?"~":"") << "x" << var(c[i]);
    }
    std::cout << std::endl;
    clauses.push();
    c.copyTo(clauses.last());
}

void Checker::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    //**/printf("%d: CHAIN %d", clauses.size(), cs[0]); for (int i = 0; i < xs.size(); i++) printf(" [%d] %d", xs[i]+1, cs[i+1]);
    std::cout << clauses.size() << ": CHAIN " << cs[0];
    for (int i = 0; i < xs.size(); i++) {
        std::cout << " [" << "x" << xs[i] << "] " << cs[i+1];
    }
    clauses.push();
    vec<Lit>& c = clauses.last();
    clauses[cs[0]].copyTo(c);
    for (int i = 0; i < xs.size(); i++)
        resolve(c, clauses[cs[i+1]], xs[i]);
    // printf(" =>"); for (int i = 0; i < c.size(); i++) printf(" %s%d", sign(c[i])?"-":"", var(c[i])+1); printf("\n");
    std::cout << " =>";
    for (int i = 0; i < c.size(); i++) {
        std::cout << " " << (sign(c[i])?"~":"") << "x" << var(c[i]);
    }
    std::cout << std::endl;
}

void Checker::deleted(ClauseId c) {
    clauses[c].clear();
}

void checkProof(Proof* proof, ClauseId goal)
{
    Checker trav;
    proof->traverse(trav, goal);

    vec<Lit>& c = trav.clauses.last();
    printf("Final clause:");
    if (c.size() == 0)
        printf(" <empty>\n");
    else{
        for (int i = 0; i < c.size(); i++)
            printf(" %s%d", sign(c[i])?"-":"", var(c[i])+1);
        printf("\n");
    }
}
