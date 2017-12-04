#include "currentproject.h"

BOOL ensureProjectSaved(void) {
    return projectSaved || unsavedProjectAlert();
}

void clearProject(void) {
    closeAllMapEditors();
    freeTilesetPackage(tilesetPackage);
    tilesetPackage = NULL;
    freeProject(&project);
    projectSaved = 1;
}

void setProjectFilename(char *filename) {
    if(filename) {
        strcpy(projectFilename, filename);
        OnMenu(projectWindow, REVERT_PROJECT_MENU_ITEM);
    } else {
        projectFilename[0] = '\0';
        OffMenu(projectWindow, REVERT_PROJECT_MENU_ITEM);
    }
}

static void openProjectFromFile(char *file) {
    Project *myNewProject;

    myNewProject = malloc(sizeof(Project));
    if(!myNewProject) {
        fprintf(stderr, "openProjectFromFile: failed to allocate project\n");
        goto done;
    }

    if(!loadProjectFromFile(file, myNewProject)) {
        EasyRequest(
            projectWindow,
            &projectLoadFailEasyStruct,
            NULL,
            file);
        goto freeProject;
    }

    clearProject();
    copyProject(myNewProject, &project);
    setProjectFilename(file);

    if(*project.tilesetPackagePath && !loadTilesetPackageFromFile(project.tilesetPackagePath)) {
        EasyRequest(
            projectWindow,
            &tilesetPackageLoadFailEasyStruct,
            NULL,
            project.tilesetPackagePath);

        /* because the tileset will now be empty, we've changed from the
           saved version */
        projectSaved = 0;
    }

freeProject:
    free(myNewProject);
done:
    return;
}

void openProjectFromAsl(char *dir, char *file) {
    size_t bufferLen = strlen(dir) + strlen(file) + 2;
    char *buffer = malloc(bufferLen);

    if(!buffer) {
        fprintf(
            stderr,
            "openProjectFromAsl: failed to allocate buffer "
            "(dir: %s) (file: %s)\n",
            dir  ? dir  : "NULL",
            file ? file : "NULL");
        goto done;
    }

    strcpy(buffer, dir);
    if(!AddPart(buffer, file, (ULONG)bufferLen)) {
        fprintf(
            stderr,
            "openProjectFromAsl: failed to add part "
            "(buffer: %s) (file: %s) (len: %d)\n",
            buffer ? buffer : "NULL",
            file   ? file   : "NULL",
            bufferLen);
        goto freeBuffer;
    }

    openProjectFromFile(buffer);

freeBuffer:
    free(buffer);
done:
    return;
}

int saveProjectToAsl(char *dir, char *file) {
    int result;
    size_t bufferLen = strlen(dir) + strlen(file) + 2;
    char *buffer = malloc(bufferLen);

    if(!buffer) {
        fprintf(
            stderr,
            "saveProjectToAsl: failed to allocate buffer "
            "(dir: %s) (file: %s)\n",
            dir  ? dir  : "NULL",
            file ? file : "NULL");
        result = 0;
        goto done;
    }

    strcpy(buffer, dir);
    if(!AddPart(buffer, file, (ULONG)bufferLen)) {
        fprintf(
            stderr,
            "saveProjectToAsl: failed to add part "
            "(buffer: %s) (file: %s) (len: %d)\n",
            buffer ? buffer : "NULL",
            file   ? file   : "NULL",
            bufferLen);
        result = 0;
        goto freeBuffer;
    }

    if(!saveProjectToFile(buffer)) {
        EasyRequest(projectWindow,
            &projectSaveFailEasyStruct,
            NULL,
            buffer);
        result = 0;
        goto freeBuffer;
    }
    setProjectFilename(buffer);

    projectSaved = 1;
    result = 1;

freeBuffer:
    free(buffer);
done:
    return result;
}
