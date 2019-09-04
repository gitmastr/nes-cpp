#pragma once
#include "types.hpp"

class Autotest
{
public:
    Autotest(const string &testName,
             const string &fileName,              
             const u32 success_display_hash);
private:
    const string testName;
    const string fileName;
    const u32 success_display_hash;
    
};