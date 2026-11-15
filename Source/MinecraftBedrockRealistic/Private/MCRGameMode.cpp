#include "MCRGameMode.h"
#include "MCRPlayerCharacter.h"

AMCRGameMode::AMCRGameMode()
{
    DefaultPawnClass = AMCRPlayerCharacter::StaticClass();
}
