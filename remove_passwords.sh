find . -type f -name \* | xargs sed -i 's@\(^//\|^\)const char\* \(ssid\|password\).*@\1const char* \2 = "\2"@g'
