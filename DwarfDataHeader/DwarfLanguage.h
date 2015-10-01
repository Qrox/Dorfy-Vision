#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_LANGUAGE_H__
#define __DORFY_VISION__DWARF_LANGUAGE_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    union LanguageRaw {
        __at(0, String identifier);
        __at(1C, String noun_singular);
        __at(38, String noun_plural);
        __at(54, String adjective);
        __at(70, String prefix);
        __at(8C, String verb_indef); // indefinite tense
        __at(A8, String verb_3rd); // third person singular
        __at(C4, String verb_past);
        __at(E0, String verb_pp); // past participle
        __at(FC, String verb_ppr); // present participle
        __at(118, dword flags);
    };

    union RaceLanguageRaw {
        __at(0, String race_identifier);
        __at(3C, Set<String *> vocabulary);
    };

    union LanguageSymRaw {
        __at(0, String identifier);
        __at(2C, Set<dword> vocabulary_index);
    };
}

#endif

#endif
