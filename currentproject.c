#include "currentproject.h"

#include <libraries/asl.h>
#include <proto/asl.h>

#include <libraries/dos.h>
#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

#include "currenttiles.h"
#include "easystructs.h"
#include "globals.h"
#include "mapeditorset.h"
#include "menu.h"
#include "ProjectWindow.h"

#define PROJECT_FILENAME_LENGTH 256

Project project;
int     projectSaved = 1;
char    projectFilename[PROJECT_FILENAME_LENGTH];

static int saveProjectToAsl(char *dir, char *file) {
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
        EasyRequest(
            getProjectWindow(),
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

int saveProjectAs(void) {
    BOOL result;
    struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
        ASL_Hail, "Save Project As",
        ASL_Window, getProjectWindow(),
        ASL_FuncFlags, FILF_SAVE,
        TAG_END);
    if(!request) {
        result = 0;
        goto done;
    }

    result = AslRequest(request, NULL);
    if(result) {
        result = saveProjectToAsl(request->rf_Dir, request->rf_File);
    }

    FreeAslRequest(request);
done:
    return result;
}

int saveProject(void) {
    if(*projectFilename) {
        if(!saveProjectToFile(projectFilename)) {
            EasyRequest(
                getProjectWindow(),
                &projectSaveFailEasyStruct,
                NULL,
                projectFilename);
            return 0;
        }
        return 1;
    } else {
        return saveProjectAs();
    }
}

static int unsavedProjectAlert(void) {
    int response = EasyRequest(
        getProjectWindow(),
        &unsavedProjectAlertEasyStruct,
        NULL);

    switch(response) {
        case 0: return 0;
        case 1: return saveProject();
        case 2: return 1;
        default:
            fprintf(stderr, "unsavedProjectAlert: unknown response %d\n", response);
            return 0;
    }
}

BOOL ensureProjectSaved(void) {
    return (BOOL)(projectSaved || unsavedProjectAlert());
}

void clearProject(void) {
    closeAllMapEditors();
    freeTilesetPackage(tilesetPackage);
    tilesetPackage = NULL;
    freeProject(&project);
    projectSaved = 1;
}

void setProjectFilename(char *filename) {
    /* TODO: honestly these OnMenu/OffMenu things could probably be made into functions elsewhere */
    if(filename) {
        strcpy(projectFilename, filename);
        OnMenu(getProjectWindow(), REVERT_PROJECT_MENU_ITEM);
    } else {
        projectFilename[0] = '\0';
        OffMenu(getProjectWindow(), REVERT_PROJECT_MENU_ITEM);
    }
}

char *getProjectFilename(void) {
    return projectFilename;
}

void openProjectFromFile(char *file) {
    Project *myNewProject;

    myNewProject = malloc(sizeof(Project));
    if(!myNewProject) {
        fprintf(stderr, "openProjectFromFile: failed to allocate project\n");
        goto done;
    }

    if(!loadProjectFromFile(file, myNewProject)) {
        EasyRequest(
            getProjectWindow(),
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
            getProjectWindow(),
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
