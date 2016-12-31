#include "Entity.h"

void entityAddNewTag(Entity *entity) {
    int newTagIdx = entity->tagCnt;

    entity->tags[newTagIdx].id = 1;
    entity->tags[newTagIdx].value = 0;
    entity->tags[newTagIdx].alias[0] = '\0';

    entity->tagCnt++;
}
