#ifndef _TEXT_MAN
#define _TEXT_MAN

#include <string>

bool isBlankChar(char c) {
    if((!isalnum(c)) && (c != '-') && (c != '_') && (c != '\r') && (c != '\n')) return true;
    return false;
}

std::string ltrimStr(std::string str) {
  int i;
  for(i=0;i<str.length();i++){
    if(((str[i] >=48) &&(str[i] <=57)) || ((str[i] >= 65) && (str[i] <= 90)) || ((str[i] >= 97) && (str[i] <= 122))) {
      break;
    }
  }

  std::string trimmed = str.substr(i, str.length()-i);
  return trimmed;
}

std::string rtrimStr(std::string str) {
    int i;
    int space = -1;

    for(i=0;i<str.length();i++) {
        if((isBlankChar(str[i])) && (space == -1)) {
            space = i;
        } else if(isalnum(str[i])) {
           space = -1;
        }

    }

    if(space != -1)
        str = str.substr(0, space);

    return str;
}

#endif
