
extern void *calloc(long __nmemb, long __size);
extern void free(void *__ptr);
extern int printf(const char *__format, ...);
#define NULL 0

typedef struct Map Map;

struct Map {
    int key;
    int value;
    Map *left;
    Map *right;
};

Map *new_map(int key, int value) {
    Map *map = calloc(1, sizeof(Map));
    map->key = key;
    map->value = value;
    return map;
}

Map *search_min(Map *map) {
    Map *cur = map;
    for (;cur->left;cur = cur->left);
    return cur;
}

void upsert(Map **map, int key, int value) {
    if (key == (*map)->key) {
        (*map)->value = value;
    } else if (key < (*map)->key) {
        if ((*map)->left) 
            upsert(&(*map)->left, key, value);
        else
            (*map)->left = new_map(key, value);
    } else {
        if ((*map)->right)
            upsert(&(*map)->right, key, value);
        else 
            (*map)->right = new_map(key, value);
    }
    return;
}

void delete(Map **map, int key) {
    if (!(*map)) 
        return;
    if (key == (*map)->key) {

        if ((*map)->right) {
            Map *min_node = search_min((*map)->right);
            (*map)->key   = min_node->key;
            (*map)->value = min_node->value;
            delete(&(*map)->right, min_node->key);
        } else {
            Map *temp = *map;
            *map = (*map)->left;
            free(temp);
        }

    } else if (key < (*map)->key) {
        delete(&(*map)->left, key);
    } else {
        delete(&(*map)->right, key);
    }
    return;
}

void dump(Map *map) {
    if (!map) 
        return;
    dump(map->left);
    printf("(key, value) = %d %d\n", map->key, map->value);
    dump(map->right);
    return;
}


int main() {

    int n = 10;

    // insert (i, i)
    Map *map = new_map(0, 0);
    for (int i = 1;i < n; i++) {
        upsert(&map, i, i);
    }
    dump(map);

    // update (i, 2*i)
    for (int i = 0;i < n; i++) {
        upsert(&map, i, 2*i);
    }
    dump(map);

    // delete even number
    for (int i = 0;i < n; i+=2) {
        delete(&map, i);
    }
    dump(map);

    return 0;
}