# HashLife C Implementation ⚡

Implémentation simple en C de l’algorithme **HashLife** pour accélérer le calcul du jeu de la vie.

## ⚠️ Avertissement

Ce projet est avant tout **éducatif** 🎓 : il vise à comprendre et expérimenter les concepts derrière HashLife (hash-consing, mémoïsation, récursion).

Il n’est **pas optimisé pour un usage sérieux ou intensif**.  
Pour des performances réelles, une gestion mémoire avancée et des fonctionnalités complètes, il est fortement recommandé d’utiliser un outil dédié comme **Golly** 🚀.


## 📦 Structure

```
hashlife/
├── bin/
├── headers/
│   ├── hashlife.h
│   └── stb_ds.h
├── src/
│   └── hashlife.c
├── main.c
├── makefile
└── readme.md

```

---

## 🚀 Compilation

```bash
make && ./bin/main
```

## 🧠 Concept

Chaque grille est représentée comme une **MacroCell** :

- niveau `n`
- taille = `2^n × 2^n`
- soit :
  - une feuille (`4×4`)
  - soit 4 sous-cellules

Les cellules sont **hash-consées** :
➡️ aucune duplication mémoire  
➡️ partage maximal des structures

---

## 🔧 API

```c
void hl_reset();

cell_id_t hl_cell_from_grid(bool* grid, size_t size);
void hl_cell_to_grid(cell_id_t id, bool* grid, size_t size);

cell_id_t hl_double_wrap(cell_id_t id);
cell_id_t hl_double_empty(cell_id_t id);

cell_id_t hl_get_result(cell_id_t id);

void hl_print_cell(cell_id_t id);
size_t hl_memory_usage();
```

## 🧩 Dépendances

- `stb_ds.h`


## ⚠️ Limitations

- Non thread-safe ❌
- Pas de GC (mémoire monotone)
- Peut consommer beaucoup de RAM sur gros patterns ⚠️


## 📜 Licence

Ce projet est distribué sous une licence libre (par exemple MIT) 🪶, ce qui signifie que vous pouvez l’utiliser, le modifier et le redistribuer librement.

Voir le fichier `LICENSE` pour les détails complets.
