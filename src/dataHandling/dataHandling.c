#include "dataHandling.h"
#include "triggers.h"
#include "physac.h"
#include "levelObjects.h"

// --- Helpers ---

float parseFloat(const char* input){
    return (float)atof(input);
}

bool isDigit(char ch){
    return ch >= '0' && ch <= '9';
}
bool isAlpha(char ch){
    return (ch >= 'a' && ch <= 'z') || 
           (ch >= 'A' && ch <= 'Z') ||
           (ch == '_');
}
bool isAlphaNumeric(char ch){
    return isAlpha(ch) || isDigit(ch);
}

// --- Scanning ---

static char ch = '\0'; // character selected
static char peek = '\0'; // next character selected
static FILE* fp = NULL;
static int line = 1;
static TokenInfo* tokenInfo = NULL;

void advanceChar(){
    ch = peek;
    if(peek != EOF)
        peek = fgetc(fp);
}

void readString(TokenInfo* tokenInfo){
    tokenInfo->type = STRING;
    int stringSize = 8;
    tokenInfo->text = (char*)calloc(stringSize, sizeof(char));
    for(int i = 0; peek != '"'; i++){
        if(i == (stringSize - 1)){
            stringSize *= 2;
            tokenInfo->text = (char*)realloc(tokenInfo->text, sizeof(char) * stringSize);
        }
        advanceChar();
        tokenInfo->text[i] = ch;
        if(peek == '"' || peek == EOF){
            i++;
            tokenInfo->text[i] = '\0';
            if(peek != EOF){
                //advanceChar();
            }else{
                TraceLog(LOG_WARNING, "readFileSF[scan/readString] - Unterminated string at end of file");
                break;
            }
        }
        if(peek == '\\'){
            advanceChar();
            advanceChar();
            i++;
            switch(ch){
                case 'n':  tokenInfo->text[i] = '\n'; break;
                case 't':  tokenInfo->text[i] = '\t'; break;
                case '\\': tokenInfo->text[i] = '\\'; break;
                case '"':  tokenInfo->text[i] = '"';  break;
                default:   tokenInfo->text[i] = '\\'; break;
            }
        }
    }
    advanceChar();
}
void readNumber(TokenInfo* tokenInfo){
    char* string;
    int stringSize = 8;
    tokenInfo->type = INTEGER;
    string = (char*)calloc(stringSize, sizeof(char));
    string[0] = ch;
    int i = 1;
    if(peek == '.'){
        i++;
        advanceChar();
        string[i] = ch;
        tokenInfo->type = FLOAT;
    }
    for(; isDigit(peek); i++){
        if(i == stringSize){
            stringSize *= 2;
            string = (char*)realloc(string, sizeof(char) * stringSize);
        }
        advanceChar();
        string[i] = ch;
        if(peek == '.'){
            i++;
            advanceChar();
            string[i] = ch;
            tokenInfo->type = FLOAT;
        }
    }
    if(tokenInfo->type == INTEGER){
        tokenInfo->integer = TextToInteger(string);
    }else{
        tokenInfo->decimal = parseFloat(string);
    }
    free(string);
}
void readKeyword(TokenInfo* tokenInfo){
    tokenInfo->type = UNDEFINED;
    int stringSize = 8;
    tokenInfo->text = (char*)calloc(stringSize, sizeof(char));
    tokenInfo->text[0] = ch;
    int i = 1;
    while(isAlphaNumeric(peek)){
        if(i == (stringSize - 1)){
            stringSize *= 2;
            tokenInfo->text = (char*)realloc(tokenInfo->text, sizeof(char) * stringSize);
        }
        advanceChar();
        tokenInfo->text[i] = ch;
        i++;
    }
    tokenInfo->text[i] = '\0';
    TraceLog(LOG_DEBUG, "readKeyword - %s", tokenInfo->text); 

    int tempVal = 0;
    if(TextIsEqual(tokenInfo->text, "true") || TextIsEqual(tokenInfo->text, "STATIC")){
        tokenInfo->type = TRUE;
    }else if(TextIsEqual(tokenInfo->text, "false") || TextIsEqual(tokenInfo->text, "DYNAMIC")){
        tokenInfo->type = FALSE;
    }else if(TextIsEqual(tokenInfo->text, "Vector2")){
        tokenInfo->type = VECTOR2;
    }else if(TextIsEqual(tokenInfo->text, "Rectangle")){
        tokenInfo->type = RECTANGLE;
    }else if(TextIsEqual(tokenInfo->text, "CirclePhysObj")){
        tokenInfo->type = CIRCLE_PHYSOBJ;
    }else if(TextIsEqual(tokenInfo->text, "RectPhysObj")){
        tokenInfo->type = RECTANGLE_PHYSOBJ;
    }else if(TextIsEqual(tokenInfo->text, "TextTrigger")){
        tokenInfo->type = TEXT_TRIGGER;
    }else if(TextIsEqual(tokenInfo->text, "Button")){
        tokenInfo->type = BUTTON;
    }else if(TextIsEqual(tokenInfo->text, "Door")){
        tokenInfo->type = DOOR;
    }else if(TextIsEqual(tokenInfo->text, "Portal")){
        tokenInfo->type = PORTAL;
    }else if(TextIsEqual(tokenInfo->text, "Wire")){
        tokenInfo->type = WIRE;
    }else{
        TraceLog(LOG_WARNING, "readFileSF[scan/readKeyword] - [line %d] Item '%s' doesn't match any keywords ", line, tokenInfo->text);
    }
    free(tokenInfo->text);
    tokenInfo->text = NULL;
    if(tokenInfo->type == ITEMTYPE) tokenInfo->integer = tempVal;
}

// --- Grouping --- 
void advanceToken(){
    tokenInfo = tokenInfo->next;
}

StructGroup* sg_alloc(){
    return (StructGroup*)calloc(1, sizeof(StructGroup));
}
// push struct group to struct group linked list
void pushGroup(StructGroup* group, StructGroup* item){
    if(group == NULL){
        TraceLog(LOG_DEBUG, "MISC-WARNING: dataHandling[readFileSF/group/addGroup] - Attempted adding to NULL group. Skipping...");
    }
    if(group->next == NULL){
        group->next = item;
    }
    StructGroup* temp;
    temp = group->next;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = item;
}


StructGroup* readFileSF(const char* path){
    TraceLog(LOG_INFO, "readFileSF - Attemping to open: %s", path);
    fp = fopen(path, "r");
    if(fp == NULL){
        TraceLog(LOG_ERROR, "readFileSF - ERROR opening. File may not exist.");
        TraceLog(LOG_ERROR, "readFileSF - Could not open: %s", path);
        return NULL;
    }
    ch = '\0'; // character selected
    peek = '\0'; // next character selected
    line = 1;
    tokenInfo = NULL;

    bool isEnd = false; // do we end read?

    // ----------------
    // readFileSF[scan]
    // ----------------
    TraceLog(LOG_INFO, "Initialize readFileSF[scan]");
    TokenInfo* infoRoot;
    infoRoot = (TokenInfo*)calloc(1, sizeof(TokenInfo));

    tokenInfo = infoRoot;
    tokenInfo->type = UNDEFINED;
    tokenInfo->next = NULL;
    peek = fgetc(fp);

    while(!isEnd){
        advanceChar();
        switch (ch){
            case '[': case '(': tokenInfo->type = LEFT_PAREN;     break;
            case ']': case ')': tokenInfo->type = RIGHT_PAREN;    break;

            case '~': tokenInfo->type = PROPERTY; break;
            case '-': tokenInfo->type = MINUS;    break;
            case ',': /* ignore */ break;

            case ' ':
            case '\r':
            case '\t':
                // lol you're irrelevent, ignoring whitespace
                break;
            
            case '\n':
                line++;
                break;
            case EOF:
                isEnd = true;
                break;

            case '/':
                while(peek != EOF && peek != '\n') advanceChar();
                break;

            case '"':
                readString(tokenInfo);
                break;
            default:
                if(isDigit(ch)){
                    readNumber(tokenInfo);
                }else if(isAlphaNumeric(ch)){
                    readKeyword(tokenInfo);
                }else{
                    TraceLog(LOG_WARNING, "readFileSF[scan] - [line %d] Unexpected character: [%c]", line, ch);
                }
                break;
        }
        tokenInfo->line = line;

        if(!isEnd && tokenInfo->type != UNDEFINED){
            tokenInfo->next = (TokenInfo*)calloc(1, sizeof(TokenInfo));
            tokenInfo = tokenInfo->next;
            tokenInfo->type = UNDEFINED;
            tokenInfo->next = NULL;
        }
    }
    fclose(fp);
    tokenInfo = infoRoot;
    if(tokenInfo != NULL && tokenInfo->next != NULL){
        while (tokenInfo->next->next != NULL) tokenInfo = tokenInfo->next;
        free(tokenInfo->next);
        tokenInfo->next = NULL;
    }

    #ifdef DEBUG_DATA_HANDLING
    TraceLog(LOG_DEBUG, "readFileSF[scan] - Begin info print at %p", infoRoot);
    tokenInfo = infoRoot;
    while(tokenInfo != NULL){
        if(tokenInfo->type == INTEGER){
            TraceLog(LOG_DEBUG, "  type: %d | line: %d | int: %d | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->integer, tokenInfo);
        }
        else if(tokenInfo->type == FLOAT){
            TraceLog(LOG_DEBUG, "  type: %d | line: %d | float: %f | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->decimal, tokenInfo);
        }else{
            TraceLog(LOG_DEBUG, "  type: %d | line: %d | text: %s | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->text, tokenInfo);
        }
        tokenInfo = tokenInfo->next;
    }
    #endif

    // -----------------
    // readFileSF[group]
    // -----------------
    StructGroup* temp;
    TraceLog(LOG_INFO, "Beginning readFileSF[group]");

    StructGroup* groupRoot;
    groupRoot = sg_alloc();

    StructGroup* structGroup = groupRoot;

    tokenInfo = infoRoot;
    bool skipAdd = false;
    bool alreadyClean = false;
    while(tokenInfo != NULL){
        skipAdd = false;
        alreadyClean = false;
        //TraceLog("token: %d", tokenInfo->type);
        if(tokenInfo->type == LEFT_PAREN){
            //TraceLog("child");
            structGroup->token.line = tokenInfo->line;
            structGroup->token.type = NO_TYPE;
            structGroup->child = sg_alloc();
            structGroup->child->parent = structGroup;
            structGroup = structGroup->child;
            skipAdd = true;
            alreadyClean = true;
            //TraceLog("parent: %p", structGroup->parent);
        }else if(tokenInfo->type == RIGHT_PAREN){
            if(structGroup->parent == NULL){
                TraceLog(LOG_ERROR, "readFileSF[group] - [line %d] Grouping failed. No opening bracket was put before a closing bracket", tokenInfo->line);
                return NULL;
            }
            structGroup->previous->next = NULL;
            temp = structGroup;
            structGroup = structGroup->parent;
            free(temp);
            skipAdd = true;
        }
        if(!skipAdd) structGroup->token = *tokenInfo;


        advanceToken();
        if(!alreadyClean){
            structGroup->next = sg_alloc();
            structGroup->next->parent = structGroup->parent;
            structGroup->next->previous = structGroup;
            structGroup = structGroup->next;
        }
        
    }

    #ifdef DEBUG_DATA_HANDLING
    structGroup = groupRoot;
    for(int i = 0; structGroup != NULL;){
        TraceLog(LOG_DEBUG, "---GROUP %d---", i);
        tokenInfo = &structGroup->token;
        if(tokenInfo->type == INTEGER){
            TraceLog(LOG_DEBUG, "type: %d | line: %d | int: %d | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->integer, tokenInfo);
        }
        else if(tokenInfo->type == FLOAT){
            TraceLog(LOG_DEBUG, "type: %d | line: %d | float: %f | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->decimal, tokenInfo);
        }else{
            TraceLog(LOG_DEBUG, "type: %d | line: %d | text: %s | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->text, tokenInfo);
        }
        if(structGroup->child != NULL){
            TraceLog(LOG_DEBUG, "\nvv- ENTER CHILD -vv\n");
            structGroup = structGroup->child;
        }else if(structGroup->next != NULL){
            structGroup = structGroup->next;
        }else{
            TraceLog(LOG_DEBUG, "\n^^- REENTER PARENT -^^\n");
            while (structGroup != NULL && structGroup->next == NULL)
            {
                structGroup = structGroup->parent;
                if(structGroup != NULL && structGroup->next != NULL){
                    structGroup = structGroup->next;
                } 
            }
        }
        i++;
    }
    #endif
    
    TraceLog(LOG_DEBUG, "readFileSF[group] - Done using [scan] materials. Freeing...");
    tokenInfo = infoRoot;
    while(tokenInfo != NULL){
        TokenInfo* safeItem = tokenInfo;
        tokenInfo = tokenInfo->next;
        //if(tokenInfo->text != NULL && tokenInfo->type != INTEGER && tokenInfo->type != FLOAT) free(tokenInfo->text); 
            // ^^ won't this crash the program because we just ref this pointer in the structGroup?
        free(safeItem);
    }
    TraceLog(LOG_DEBUG, "readFileSF[group] - Successfully freed all old tokens");

    
    // -----------------
    // readFileSF[associate]
    // -----------------
    TraceLog(LOG_INFO, "Moving onto readFileSF[associate]...");
    
    structGroup = groupRoot;
    bool success = false;
    while (structGroup != NULL)
    {
        success = false;
        //TraceLog("i %d", structGroup->token.type);
        if(structGroup->token.type > RIGHT_PAREN){
            //should be associated
            if(structGroup->next->token.type == NO_TYPE){
                structGroup->next->token = structGroup->token;

                success = true;
            }else{
                TraceLog(LOG_DEBUG, "readFileSF[associate] - token: %d", structGroup->token.type);
                TraceLog(LOG_DEBUG, "readFileSF[associate] - next_token: %d", structGroup->next->token.type);
                TraceLog(LOG_ERROR, "readFileSF[associate] - [line %d] Expected group but got item instead (did you forget parentheses?)", structGroup->token.line);
                return NULL;
            }
        }else if(structGroup->token.type == MINUS){
            if(structGroup->next->token.type == INTEGER){
                structGroup->next->token.integer = -structGroup->next->token.integer;
            }else if(structGroup->next->token.type == FLOAT){
                structGroup->next->token.decimal = -structGroup->next->token.decimal;
            }else{
                TraceLog(LOG_ERROR, "readFileSF[associate] - [line %d] Expected number while reading a negative symbol", structGroup->token.line);
                return NULL;
            }
            success = true;
        }
        if(success){
            if(structGroup->previous != NULL)structGroup->previous->next = structGroup->next;
            if(structGroup == groupRoot) groupRoot = structGroup->next;
            if(structGroup->parent != NULL && structGroup == structGroup->parent->child) structGroup->parent->child = structGroup->next;
            structGroup->next->previous = structGroup->previous;
            temp = structGroup->next;
            free(structGroup);
            structGroup = temp;
        }
        //TraceLog("i %d", structGroup->token.type);
        if(structGroup->child != NULL){
            //TraceLog("vv- FOUND CHILD. ENTER CHILD -vv");
            structGroup = structGroup->child;
        }else if(structGroup->next != NULL){
            structGroup = structGroup->next;
        }else{
            //TraceLog("^^- REENTER PARENT -^^");
            while (structGroup != NULL && structGroup->next == NULL)
            {
                structGroup = structGroup->parent;
                if(structGroup != NULL && structGroup->next != NULL){
                    structGroup = structGroup->next;
                } 
            }
        }
    }
    #ifdef DEBUG_DATA_HANDLING
    structGroup = groupRoot;
    for(int i = 0; structGroup != NULL;){
        TraceLog(LOG_DEBUG, "---GROUP %d---", i);
        tokenInfo = &structGroup->token;
        if(tokenInfo->type == INTEGER){
            TraceLog(LOG_DEBUG, "type: %d | line: %d | int: %d | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->integer, tokenInfo);
        }
        else if(tokenInfo->type == FLOAT){
            TraceLog(LOG_DEBUG, "type: %d | line: %d | float: %f | loc: %p", tokenInfo->type, tokenInfo->line, tokenInfo->decimal, tokenInfo);
        }else{
            TraceLog(LOG_DEBUG, "type: %d | line: %d | text: DISABLED | loc: %p", tokenInfo->type, tokenInfo->line, /*tokenInfo->text,*/ tokenInfo);
        }
        if(structGroup->child != NULL){
            TraceLog(LOG_DEBUG, "\nvv- ENTER CHILD -vv\n");
            structGroup = structGroup->child;
        }else if(structGroup->next != NULL){
            structGroup = structGroup->next;
        }else{
            TraceLog(LOG_DEBUG, "\n^^- REENTER PARENT -^^\n");
            while (structGroup != NULL && structGroup->next == NULL)
            {
                structGroup = structGroup->parent;
                if(structGroup != NULL && structGroup->next != NULL){
                    structGroup = structGroup->next;
                } 
            }
        }
        i++;
    }
    #endif
    return groupRoot;
}

// --- Parsing --- 

//does not include grandchildren, only direct children
int childNum(StructGroup* group){
    //Fucntion panics
        if(group == NULL) return -1;

    StructGroup* item = group->child;
    int i = 0;
    while(item != NULL){
        item = item->next;
        i++;
    }
    return i;
}

//check if a group has enough args to be satisfied
//if true; that means there's an error
bool checkArgNumber(StructGroup* group, int expectedArgs, const char* itemName){
    if(childNum(group) != expectedArgs){
        TraceLog(LOG_ERROR, "parseStructGroupInfo[checkArgNumber] - [line %d] Unexpected number of arguments given to %s", group->token.line, itemName);
        return true;
    }else{
        return false;
    }
}

//raylib types
Vector2 parseVector2(StructGroup* group){
    Vector2 vector2 = {0, 0};
    if(checkArgNumber(group, 2, "Vector2")) return vector2;
    
    StructGroup* temp = group->child;
    for(int i = 0; i < 2 && temp != NULL; i++){
        if(temp->token.type == INTEGER){
            if(i == 0) vector2.x = temp->token.integer;
            else vector2.y = temp->token.integer;
        }else if(temp->token.type == FLOAT){
            if(i == 0) vector2.x = temp->token.decimal;
            else vector2.y = temp->token.decimal;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parseVector2] - [line %d] Received non-number argument. Argument skipped.", temp->token.line);
        }
        temp = temp->next;
    }
    return vector2;
}
Rectangle parseRectangle(StructGroup* group){
    Rectangle rect = {0, 0, 0, 0};
    if(checkArgNumber(group, 4, "Rectangle")) return rect;;
    
    StructGroup* temp = group->child;
    for(int i = 0; i < 4 && temp != NULL; i++){
        if(temp->token.type == INTEGER){
            if(i == 0) rect.x = temp->token.integer;
            if(i == 1) rect.y = temp->token.integer;
            if(i == 2) rect.width = temp->token.integer;
            else rect.height = temp->token.integer;
        }else if(temp->token.type == FLOAT){
            if(i == 0) rect.x = temp->token.decimal;
            if(i == 1) rect.y = temp->token.decimal;
            if(i == 2) rect.width = temp->token.decimal;
            else rect.height = temp->token.decimal;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parseRectangle] - [line %d] Received non-number argument. Argument skipped.", temp->token.line);
        }
        temp = temp->next;
    }
    return rect;
}

//structs

TempPhysObj parsePhysObj(StructGroup* group, bool isCircle){
    TempPhysObj obj = {0};
    //TraceLog("arg num: %d", childNum(group));
    if(isCircle && checkArgNumber(group, 5, "CirclePhysObj")) return obj;
    if(!isCircle && checkArgNumber(group, 6, "RectanglePhysObj")) return obj;

    StructGroup* temp = group->child;
    
    if(temp->token.type == VECTOR2){
        obj.pos = parseVector2(temp);
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Invalid argument type for arg 1", group->token.line);
    }
    temp = temp->next;
    if(isCircle){
        if(temp->token.type == FLOAT){
            obj.radius = temp->token.decimal;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parseCirclePhysObj] - [line %d] Invalid argument type for arg 2", group->token.line);
        }
    }else{
        if(temp->token.type == FLOAT){
            obj.width = temp->token.decimal;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parseRectanglePhysObj] - [line %d] Invalid argument type for arg 2", group->token.line);
        }
        temp = temp->next;
        if(temp->token.type == FLOAT){
            obj.height = temp->token.decimal;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parseRectanglePhysObj] - [line %d] Invalid argument type for arg 3", group->token.line);
        }
    }
    temp = temp->next;
    obj.isStatic = temp->token.type == TRUE;
    temp = temp->next;
    if(temp->token.type == NO_TYPE){
        obj.tagCount = 0;
        int tagCount = 8;
        obj.tags = malloc(tagCount * sizeof(unsigned int));
        temp = temp->child;
        for(int i = 0; temp->next != NULL; i++){
            if(temp->token.type != INTEGER){
                TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Invalid type for what should be inside an INTEGER array", group->token.line);
                break;
            }
            if(i == (tagCount - 1)){
                tagCount *= 2;
                obj.tags = realloc(obj.tags, sizeof(unsigned int) * tagCount);
            }
            obj.tags[i] = temp->token.integer;
            obj.tagCount++;
            temp = temp->next;
        }
        if(temp->token.type != INTEGER){
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Invalid type for what should be inside an INTEGER array", group->token.line);
        }else{
            if(obj.tagCount == (tagCount - 1)){
                tagCount *= 2;
                obj.tags = realloc(obj.tags, sizeof(unsigned int) * tagCount);
            }
            obj.tags[obj.tagCount] = temp->token.integer;
            obj.tagCount++;
        }
        temp = temp->parent;
    }else if(temp->token.type == INTEGER){
        obj.tagCount = 1;
        obj.tags = malloc(sizeof(unsigned int));
        obj.tags[0] = temp->token.integer;
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Invalid argument type for arg 2", group->token.line);
    }
    temp = temp->next;
    if(temp->token.type == INTEGER){
        if(temp->token.integer >= 0){
            obj.trigger = temp->token.integer;
        }else{
            TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Trigger cannot be negative", group->token.line);
        }
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parsePhysObj] - [line %d] Invalid argument type for last arg", group->token.line);
    }
    return obj;
}

TextBoxTrigger parseTrigger(StructGroup* group){
    TextBoxTrigger textBox = {0};
    if(checkArgNumber(group, 2, "TextBoxTrigger")) return textBox;

    StructGroup* temp = group->child;
    
    if(temp->token.type == INTEGER){
        textBox.trigger = temp->token.integer;
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseTrigger] - [line %d] Invalid argument type for arg 1", group->token.line);
    }
    temp = temp->next;
    if(temp->token.type == NO_TYPE){
        int textCount = 8;
        textBox.texts = malloc(textCount * sizeof(char*));
        temp = temp->child;
        for(int i = 0; temp != NULL; i++){
            if(temp->token.type != STRING){
                TraceLog(LOG_WARNING, "parseStructGroupInfo[parseTrigger] - [line %d] Invalid type for what should be inside a STRING array", group->token.line);
                break;
            }
            if(i == (textCount - 1)){
                textCount *= 2;
                textBox.texts = realloc(textBox.texts, sizeof(char*) * textCount);
            }
            textBox.texts[i] = calloc(TextLength(temp->token.text), sizeof(char));
            TextCopy(textBox.texts[i], temp->token.text);
            textBox.textCount++;
            temp = temp->next;
        }
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseTrigger] - [line %d] Invalid argument type for arg 2", group->token.line);
    }
    return textBox;
}

WireDoorData parseWireOrDoor(StructGroup* group){
    WireDoorData wire = {0};
    if(checkArgNumber(group, 3, "Wire")) return wire;

    StructGroup* temp = group->child;
    
    if(temp->token.type == VECTOR2){
        wire.pos = parseVector2(temp);
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseWireOrDoor] - [line %d] Invalid argument type for arg 1", group->token.line);
    }
    temp = temp->next;
    if(temp->token.type == INTEGER){
        wire.doorTypeOrWireID = temp->token.integer;
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseWireOrDoor] - [line %d] Invalid argument type for arg 2", group->token.line);
    }
    temp = temp->next;
    if(temp->token.type == INTEGER){
        wire.trigger = temp->token.integer;
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseWireOrDoor] - [line %d] Invalid argument type for arg 3", group->token.line);
    }
    return wire;
}

ButtonPortalData parseButtonOrPortal(StructGroup* group){
    ButtonPortalData button = {0};
    if(checkArgNumber(group, 2, "ButtonOrPotal")) return button;

    StructGroup* temp = group->child;
    
    if(temp->token.type == VECTOR2){
        button.pos = parseVector2(temp);
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseButtonOrPortal] - [line %d] Invalid argument type for arg 1", group->token.line);
    }
    temp = temp->next;
    if(temp->token.type == INTEGER){
        button.triggerOrColourID = temp->token.integer;
    }else{
        TraceLog(LOG_WARNING, "parseStructGroupInfo[parseButtonOrPortal] - [line %d] Invalid argument type for arg 2", group->token.line);
    }
    return button;
}


int parseStructGroupInfo(StructGroup* groupRoot, void (*function_harold_prompt)(const char** texts, int textCount), Vector2 *startingPos){
    TraceLog(LOG_INFO, "Finalizing read with parseStructGroupInfo...");

    StructGroup* structGroup = groupRoot;
    while(structGroup != NULL){
        //TraceLog("reading...");
        TempPhysObj obj;
        PhysicsBody body;
        switch (structGroup->token.type)
        {
            case CIRCLE_PHYSOBJ:
                obj = parsePhysObj(structGroup, true);
                body = CreatePhysicsBodyCircle(obj.pos, obj.radius, 1, obj.trigger);
                for(int i = 0; i < obj.tagCount; i++){
                    AddTagToPhysicsBody(body, obj.tags[i]);
                }
                body->freezeOrient = true;
                body->enabled = !obj.isStatic;
                break;
            case RECTANGLE_PHYSOBJ:
                obj = parsePhysObj(structGroup, false);
                body = CreatePhysicsBodyRectangle((Vector2){obj.pos.x + obj.width / 2, obj.pos.y + obj.height / 2}, obj.width, obj.height, 1, obj.trigger);
                for(int i = 0; i < obj.tagCount; i++){
                    AddTagToPhysicsBody(body, obj.tags[i]);
                }
                body->freezeOrient = true;
                body->enabled = !obj.isStatic;
                break;
            case TEXT_TRIGGER:; //dumb semicolon again!!! thanks emscripten!
                TextBoxTrigger textBox = parseTrigger(structGroup);
                NewTriggerEvent(textBox.trigger, TRIGGER_USE_ONCE, CreateTriggerEventFunctionData_TextPrompt((const char**)textBox.texts, textBox.textCount, function_harold_prompt));
                break;
            case PROPERTY:
                *startingPos = parseVector2(structGroup);
                break;
            case WIRE:; //dumb semicolon again!!! thanks emscripten!
                WireDoorData wire = parseWireOrDoor(structGroup);
                CreateWire(wire.pos, wire.doorTypeOrWireID, wire.trigger, false);
                break;
            case BUTTON:; //dumb semicolon again!!! thanks emscripten!
                ButtonPortalData button = parseButtonOrPortal(structGroup);
                body = CreatePhysicsBodyRectangle((Vector2){button.pos.x + 15, button.pos.y + 5.5f}, 30, 11, 1, button.triggerOrColourID);
                body->freezeOrient = true;
                body->enabled = false;
                CreateButton(button.pos, button.triggerOrColourID, false);
                //assign to render button
                break;
            case PORTAL:; //dumb semicolon again!!! thanks emscripten!
                ButtonPortalData portal = parseButtonOrPortal(structGroup);
                body = CreatePhysicsBodyCircle((Vector2){portal.pos.x + 13, portal.pos.y + 22}, 14, 1, portal.triggerOrColourID + 13);
                body->freezeOrient = true;
                body->enabled = false;
                CreatePortal(portal.pos, portal.triggerOrColourID);
                break;
            case DOOR:; //dumb semicolon again!!! thanks emscripten!
                WireDoorData door = parseWireOrDoor(structGroup);
                CreateDoor(door.pos, door.doorTypeOrWireID, door.trigger);
                break;
            default:
                TraceLog(LOG_WARNING, "parseStructGroupInfo - [line %d] Received non-struct as parent group. [TOKEN_TYPE: %d] Skipping...", structGroup->token.line, structGroup->token.type);
                break;
        }
        structGroup = structGroup->next;
    }
    return 0;
}