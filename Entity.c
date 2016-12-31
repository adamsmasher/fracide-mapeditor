#include "Entity.h"

#include <strings.h>

void entityAddNewTag(Entity *entity) {
    int newTagIdx = entity->tagCnt;

    entity->tags[newTagIdx].id = 1;
    entity->tags[newTagIdx].value = 0;
    entity->tags[newTagIdx].alias[0] = '\0';

    entity->tagCnt++;
}

void entityDeleteTag(Entity *entity, int tagNum) {
    entity->tagCnt--;

    memmove(
        &entity->tags[tagNum],
        &entity->tags[tagNum + 1],
        sizeof(Frac_tag) * (entity->tagCnt - tagNum));
}
