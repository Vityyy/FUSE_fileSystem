
# No debe existir new_file

echo -n "02_file_creation_with_redirection - "


echo -n "hola fisopfs!" > new_file

contenido=$(cat new_file)

if [[ "$contenido" == "hola fisopfs!" ]]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

rm new_file
