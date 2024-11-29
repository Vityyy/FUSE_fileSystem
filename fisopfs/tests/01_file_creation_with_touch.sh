
# No debe existir new_file

echo -n "01_file_creation_with_touch - "


cd prueba

touch new_file

ls -1 | grep -q new_file

if [ $? -eq 0 ]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

rm new_file
