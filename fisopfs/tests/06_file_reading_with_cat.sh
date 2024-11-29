
# Debe existir un archivo 'example_file' con el contenido 'hola fisopfs!'

echo -n "06_file_reading_with_cat - "


contenido=$(cat example_file)

if [[ "$contenido" == "hola fisopfs!" ]]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi
