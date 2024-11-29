
# No debe existir new_dir

echo -n "05_non_empty_dir_reading_with_ls - "


mkdir new_dir
echo -n "hola soy el archivo 1!" > new_dir/file_1
echo -n "hola soy el archivo 2!" > new_dir/file_2
mkdir new_dir/new_sub_dir
echo -n "hola soy el archivo 3!" > new_dir/new_sub_dir/file_3

num=$(ls -1 -a new_dir | wc -l)

if [[ "$num" == "5" ]]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

rm -r new_dir
