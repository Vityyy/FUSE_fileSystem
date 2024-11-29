
# Debe existir un archivo llamado 'example_file' con el contenido 'hola fisopfs!'

echo -n "07_file_overwrite_with_redirection - "


cp example_file tmp
echo -n "chau fisopfs!" > example_file
contenido=$(cat example_file)

if [[ "$contenido" == "chau fisopfs!" ]]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

cp tmp example_file
rm tmp
