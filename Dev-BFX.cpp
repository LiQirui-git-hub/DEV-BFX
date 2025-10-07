#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <fstream>
#include <cstring>
#include <map>
#include <sstream>
#include <conio.h> 

// 平台相关的头文件
#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#else
    #include <dirent.h>
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
#endif

#include <windows.h>
#include <functional>

class BrainfuckCompiler {
private:
    std::vector<uint8_t> memory;
    size_t data_pointer;
    std::string code;
    size_t instruction_pointer;
    
    // 浼樺寲锛氶璁＄畻璺宠浆浣嶇疆
    std::vector<size_t> jump_forward;
    std::vector<size_t> jump_backward;

public:
    static const size_t MEMORY_SIZE = 30000;

    BrainfuckCompiler() : memory(MEMORY_SIZE, 0), data_pointer(0), instruction_pointer(0) {
        jump_forward.resize(MEMORY_SIZE);
        jump_backward.resize(MEMORY_SIZE);
    }

    // 棰勮绠楀惊鐜烦杞綅缃?
    void precomputeJumps() {
        std::stack<size_t> loop_stack;

        for (size_t i = 0; i < code.length(); i++) {
            char c = code[i];

            if (c == '[') {
                loop_stack.push(i);
            } else if (c == ']') {
                if (loop_stack.empty()) {
                    throw std::runtime_error("Unmatched ']' at position " + std::to_string(i));
                }

                size_t start = loop_stack.top();
                loop_stack.pop();

                jump_forward[start] = i;
                jump_backward[i] = start;
            }
        }

        if (!loop_stack.empty()) {
            throw std::runtime_error("Unmatched '[' in code");
        }
    }

    // 娓呯悊鍜岄獙璇佷唬鐮?
    void loadCode(const std::string& brainfuck_code) {
        code.clear();

        // 鍙繚鐣欐湁鏁堢殑Brainfuck鎸囦护
        for (char c : brainfuck_code) {
            if (c == '>' || c == '<' || c == '+' || c == '-' ||
                c == '.' || c == ',' || c == '[' || c == ']') {
                code += c;
            }
        }

        precomputeJumps();
    }

    // 浠庢枃浠跺姞杞戒唬鐮?
    void loadCodeFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        loadCode(buffer.str());
    }

    // 瑙ｉ噴鎵ц
    void interpret() {
        data_pointer = 0;
        instruction_pointer = 0;
        fill(memory.begin(), memory.end(), 0);

        while (instruction_pointer < code.length()) {
            executeInstruction();
            instruction_pointer++;
        }
    }

    // 鍗曟鎵ц锛堢敤浜庤皟璇曪級
    bool step() {
        if (instruction_pointer >= code.length()) {
            return false;
        }

        executeInstruction();
        instruction_pointer++;
        return true;
    }

    // 鑾峰彇鍐呭瓨鐘舵€侊紙鐢ㄤ簬璋冭瘯锛?
    void printMemoryState(size_t start = 0, size_t count = 20) {
        std::cout << "Memory state (pointer at " << data_pointer << "): ";
        for (size_t i = start; i < start + count && i < memory.size(); i++) {
            std::cout << static_cast<int>(memory[i]) << " ";
        }
        std::cout << std::endl;
    }

    // 鑾峰彇褰撳墠鐘舵€侊紙鐢ㄤ簬璋冭瘯锛?
    void printCurrentState() {
        std::cout << "IP: " << instruction_pointer << ", DP: " << data_pointer
                  << ", Current Cell: " << static_cast<int>(memory[data_pointer]) << std::endl;
    }

private:
    void executeInstruction() {
        char instruction = code[instruction_pointer];
        switch (instruction) {
            case '>':
                data_pointer = (data_pointer + 1) % MEMORY_SIZE;
                break;

            case '<':
                data_pointer = (data_pointer == 0) ? MEMORY_SIZE - 1 : data_pointer - 1;
                break;

            case '+':
                memory[data_pointer]++;
                break;

            case '-':
                memory[data_pointer]--;
                break;

            case '.':
            	std::cout << "Output(only-one-character) >> " << memory[data_pointer] << "\n";
                break;

            case ',':
                {
                	std::cout << "Input(only-one-character,any-extra-is-ignored) >> ";
                    std::string input;
                    std::getline(std::cin,input);
                    memory[data_pointer] = input[0];
                }
                break;

            case '[':
                if (memory[data_pointer] == 0) {
                    instruction_pointer = jump_forward[instruction_pointer];
                }
                break;

            case ']':
                if (memory[data_pointer] != 0) {
                    instruction_pointer = jump_backward[instruction_pointer];
                }
                break;
        }
    }

public:
    // 缂栬瘧涓篊浠ｇ爜
    std::string compileToC() {
        std::stringstream c_code;

        c_code << "#include <bits/stdc++.h>\n";
        c_code << "using namespace std;\n\n";
        c_code << "int main() {\n";
        c_code << "    unsigned char memory[" << MEMORY_SIZE << "] = {0};\n";
        c_code << "    unsigned char *ptr = memory;\n\n";

        for (size_t i = 0; i < code.length(); i++) {
            char instruction = code[i];

            // 娣诲姞缂╄繘
            c_code << "    ";

            switch (instruction) {
                case '>':
                    c_code << "++ptr;";
                    break;
                case '<':
                    c_code << "--ptr;";
                    break;
                case '+':
                    c_code << "++*ptr;";
                    break;
                case '-':
                    c_code << "--*ptr;";
                    break;
                case '.':
                    c_code << "putchar(*ptr);";
                    break;
                case ',':
                    c_code << "*ptr = getchar();";
                    break;
                case '[':
                    c_code << "while (*ptr) {";
                    break;
                case ']':
                    c_code << "}";
                    break;
            }

            c_code << "\n";
        }

        c_code << "    return 0;\n";
        c_code << "}\n";

        return c_code.str();
    }

    // 缂栬瘧涓篊++浠ｇ爜
    std::string compileToCpp() {
        std::stringstream cpp_code;

        cpp_code << "#include <bits/stdc++.h>\n";
        cpp_code << "using namespace std;\n\n";
        cpp_code << "int main() {\n";
        cpp_code << "    vector<unsigned char> memory(" << MEMORY_SIZE << ", 0);\n";
        cpp_code << "    size_t data_pointer = 0;\n\n";

        for (size_t i = 0; i < code.length(); i++) {
            char instruction = code[i];

            // 娣诲姞缂╄繘
            cpp_code << "    ";

            switch (instruction) {
                case '>':
                    cpp_code << "data_pointer = (data_pointer + 1) % " << MEMORY_SIZE << ";";
                    break;
                case '<':
                    cpp_code << "data_pointer = (data_pointer == 0) ? " << MEMORY_SIZE - 1 << " : data_pointer - 1;";
                    break;
                case '+':
                    cpp_code << "memory[data_pointer]++;";
                    break;
                case '-':
                    cpp_code << "memory[data_pointer]--;";
                    break;
                case '.':
                    cpp_code << "cout << memory[data_pointer];";
                    break;
                case ',':
                    cpp_code << "cin >> memory[data_pointer];";
                    break;
                case '[':
                    cpp_code << "while (memory[data_pointer] != 0) {";
                    break;
                case ']':
                    cpp_code << "}";
                    break;
            }

            cpp_code << "\n";
        }

        cpp_code << "    return 0;\n";
        cpp_code << "}\n";

        return cpp_code.str();
    }
};

// 语言定义
enum Language {
    ENGLISH,
    CHINESE,
    SPANISH,
    FRENCH,
    GERMAN,
    JAPANESE,
    RUSSIAN,
    PORTUGUESE
};

// 颜色定义
enum Color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    ORANGE,
    BROWN
};

// 多语言文本映射
std::map<Language, std::map<std::string, std::string> > translations;

// 初始化翻译映射
void initializeTranslations() {
    // English (补全debug_program，调整命令序号)
    std::map<std::string, std::string> english;
    english["main_menu"] = "Brainfuck IDE";
    english["main_menu1"] = "Create Program";
    english["main_menu2"] = "Open Program";
    english["main_menu3"] = "Editor Settings";
    english["main_menu4"] = "Exit";
    english["editor_title"] = "=== Brainfuck Program Editor ===";
    english["current_program"] = "Current Program (Original):";
    english["filtered_code"] = "Filtered executable code:";
    english["editor_commands"] = "Editor Commands:";
    english["edit_program"] = "1. Input/Edit program (supports comments)";
    english["run_program"] = "2. Run program";
    english["debug_program"] = "3. Debug Program";  // 补全
    english["save_program"] = "4. Save program (with comments)";  // 调整序号
    english["clear_program"] = "5. Clear program";  // 调整序号
    english["show_filtered"] = "6. Show filtered code";  // 调整序号
    english["language_settings_editor"] = "7. Language Settings";  // 调整序号
    english["back_menu"] = "8. Back to main menu";  // 调整序号
    english["select_option"] = "Select: ";
    english["program_empty"] = "Program is empty!";
    english["program_cleared"] = "Program cleared!";
    english["invalid_choice"] = "Invalid choice!";
    english["enter_filename"] = "Enter filename (without extension): ";
    english["save_success"] = "Program saved successfully!";
    english["save_failed"] = "Save failed!";
    english["run_results"] = "Run Results:";
    english["run_success"] = "Program executed successfully.";
    english["pointer_error"] = "Pointer out of bounds!";
    english["compile_error"] = "Compile error!";
    english["input_program"] = "Enter Brainfuck program (characters other than the 8 valid commands and //, /* */ are treated as comments, enter '0' alone to end):";
    english["comments_supported"] = "Supports single-line (//) and multi-line (/* */) comments, other characters are also treated as comments";
    english["no_bf_files"] = "No .bf files found!";
    english["found_files"] = "Found %d .bf files (including subdirectories):";
    english["open_file_prompt"] = "Enter file number to open (0 to return): ";
    english["invalid_selection"] = "Invalid selection!";
    english["opening_file"] = "Opening file: %s";
    english["file_content"] = "Program content (with comments):";
    english["enter_to_editor"] = "Press any key to enter editor...";
    english["file_empty"] = "File is empty or read failed!";
    english["language_settings"] = "=== Editor Language Settings ===";
    english["current_language"] = "Current language: ";
    english["select_language"] = "Select language:";
    english["back_to_main"] = "Back to main menu";
    english["language_changed"] = "Language changed successfully!";
    english["editor_settings"] = "=== Editor Settings ===";
    english["color_settings"] = "=== Color Settings ===";
    english["current_background"] = "Current background color: ";
    english["current_code_color"] = "Current code color: ";
    english["current_comment_color"] = "Current comment color: ";
    english["select_background_color"] = "Select background color:";
    english["select_code_color"] = "Select code color:";
    english["select_comment_color"] = "Select comment color:";
    english["color_options"] = "1. Predefined colors\n2. Custom RGB color\n3. Back to color settings";
    english["predefined_colors"] = "Predefined colors:";
    english["enter_rgb"] = "Enter RGB values (0-255) separated by spaces (R G B): ";
    english["color_changed"] = "Color changed successfully!";
    english["invalid_rgb"] = "Invalid RGB values! Using default color.";
    english["reset_colors"] = "Reset to default colors";
    english["colors_reset"] = "Colors reset to default!";
    translations[ENGLISH] = english;
    
    // Chinese (修正序号冲突，补全分号)
    std::map<std::string, std::string> chinese;
    chinese["main_menu"] = "Brainfuck 集成开发环境";
    chinese["main_menu1"] = "创建程序";
    chinese["main_menu2"] = "打开程序";
    chinese["main_menu3"] = "编辑器设置";
    chinese["main_menu4"] = "退出";
    chinese["editor_title"] = "=== Brainfuck 程序编辑器 ===";
    chinese["current_program"] = "当前程序 (原始):";
    chinese["filtered_code"] = "过滤后的执行代码:";
    chinese["editor_commands"] = "编辑器指令:";
    chinese["edit_program"] = "1. 输入/编辑程序 (支持注释)";
    chinese["run_program"] = "2. 运行程序";
    chinese["debug_program"] = "3. 调试程序";  // 补全分号
    chinese["save_program"] = "4. 保存程序 (包含注释)";
    chinese["clear_program"] = "5. 清空程序";
    chinese["show_filtered"] = "6. 显示过滤后的代码";
    chinese["language_settings_editor"] = "7. 语言设置";  // 修正序号冲突
    chinese["back_menu"] = "8. 返回主菜单";  // 调整序号
    chinese["select_option"] = "选择: ";
    chinese["program_empty"] = "程序为空!";
    chinese["program_cleared"] = "程序已清空!";
    chinese["invalid_choice"] = "无效选择!";
    chinese["enter_filename"] = "请输入文件名(不带扩展名): ";
    chinese["save_success"] = "程序保存成功!";
    chinese["save_failed"] = "保存失败!";
    chinese["run_results"] = "运行结果:";
    chinese["run_success"] = "程序执行成功。";
    chinese["pointer_error"] = "指针越界!";
    chinese["compile_error"] = "编译错误!";
    chinese["input_program"] = "请输入Brainfuck程序 (除8种有效指令和//、/* */外的字符均视为注释，输入0单独一行结束):";
    chinese["comments_supported"] = "支持单行注释（//）和多行注释（/* */），其他字符也视为注释";
    chinese["no_bf_files"] = "没有找到任何.bf文件!";
    chinese["found_files"] = "找到 %d 个 .bf 文件 (包含子目录):";
    chinese["open_file_prompt"] = "请输入要打开的文件编号 (0返回): ";
    chinese["invalid_selection"] = "无效的选择!";
    chinese["opening_file"] = "打开文件: %s";
    chinese["file_content"] = "程序内容 (包含注释):";
    chinese["enter_to_editor"] = "按任意键进入编辑器...";
    chinese["file_empty"] = "文件为空或读取失败!";
    chinese["language_settings"] = "=== 编辑器语言设置 ===";
    chinese["current_language"] = "当前语言: ";
    chinese["select_language"] = "选择语言:";
    chinese["back_to_main"] = "返回主菜单";
    chinese["language_changed"] = "语言切换成功!";
    chinese["editor_settings"] = "=== 编辑器设置 ===";
    chinese["color_settings"] = "=== 颜色设置 ===";
    chinese["current_background"] = "当前背景颜色: ";
    chinese["current_code_color"] = "当前代码颜色: ";
    chinese["current_comment_color"] = "当前注释颜色: ";
    chinese["select_background_color"] = "选择背景颜色:";
    chinese["select_code_color"] = "选择代码颜色:";
    chinese["select_comment_color"] = "选择注释颜色:";
    chinese["color_options"] = "1. 预定义颜色\n2. 自定义RGB颜色\n3. 返回颜色设置";
    chinese["predefined_colors"] = "预定义颜色:";
    chinese["enter_rgb"] = "请输入RGB值 (0-255)，用空格分隔 (R G B): ";
    chinese["color_changed"] = "颜色修改成功!";
    chinese["invalid_rgb"] = "无效的RGB值! 使用默认颜色。";
    chinese["reset_colors"] = "重置为默认颜色";
    chinese["colors_reset"] = "颜色已重置为默认值!";
    translations[CHINESE] = chinese;
    
    // Spanish (修正倒感叹号，补全debug_program，调整序号)
    std::map<std::string, std::string> spanish;
    spanish["main_menu"] = "IDE Brainfuck";
    spanish["main_menu1"] = "Crear Programa";
    spanish["main_menu2"] = "Abrir Programa";
    spanish["main_menu3"] = "Configuración del Editor";
    spanish["main_menu4"] = "Salir";
    spanish["editor_title"] = "=== Editor de Programa Brainfuck ===";
    spanish["current_program"] = "Programa Actual (Original):";
    spanish["filtered_code"] = "Código ejecutable filtrado:";
    spanish["editor_commands"] = "Comandos del Editor:";
    spanish["edit_program"] = "1. Ingresar/Editar programa (soporta comentarios)";
    spanish["run_program"] = "2. Ejecutar programa";
    spanish["debug_program"] = "3. Depurar programa";  // 补全
    spanish["save_program"] = "4. Guardar programa (con comentarios)";  // 调整序号
    spanish["clear_program"] = "5. Limpiar programa";  // 调整序号
    spanish["show_filtered"] = "6. Mostrar código filtrado";  // 调整序号
    spanish["language_settings_editor"] = "7. Configuración de Idioma";  // 调整序号
    spanish["back_menu"] = "8. Volver al menú principal";  // 调整序号
    spanish["select_option"] = "Seleccionar: ";
    spanish["program_empty"] = "?El programa está vacío!";  // 修正倒感叹号
    spanish["program_cleared"] = "?Programa limpiado!";  // 修正倒感叹号
    spanish["invalid_choice"] = "?Elección inválida!";  // 修正倒感叹号
    spanish["enter_filename"] = "Ingrese nombre de archivo (sin extensión): ";
    spanish["save_success"] = "?Programa guardado exitosamente!";  // 修正倒感叹号
    spanish["save_failed"] = "?Guardado fallido!";  // 修正倒感叹号
    spanish["run_results"] = "Resultados de Ejecución:";
    spanish["run_success"] = "Programa ejecutado exitosamente.";
    spanish["pointer_error"] = "?Puntero fuera de límites!";  // 修正倒感叹号
    spanish["compile_error"] = "?Error de compilación!";  // 修正倒感叹号
    spanish["input_program"] = "Ingrese programa Brainfuck (caracteres distintos a los 8 comandos válidos y //, /* */ son tratados como comentarios, ingrese '0' solo para terminar):";
    spanish["comments_supported"] = "Soporta comentarios de una línea (//) y multi-línea (/* */), otros caracteres también son tratados como comentarios";
    spanish["no_bf_files"] = "?No se encontraron archivos .bf!";  // 修正倒感叹号
    spanish["found_files"] = "Encontrados %d archivos .bf (incluyendo subdirectorios):";
    spanish["open_file_prompt"] = "Ingrese número de archivo a abrir (0 para volver): ";
    spanish["invalid_selection"] = "?Selección inválida!";  // 修正倒感叹号
    spanish["opening_file"] = "Abriendo archivo: %s";
    spanish["file_content"] = "Contenido del programa (con comentarios):";
    spanish["enter_to_editor"] = "Presione cualquier tecla para entrar al editor...";
    spanish["file_empty"] = "?El archivo está vacío o la lectura falló!";  // 修正倒感叹号
    spanish["language_settings"] = "=== Configuración de Idioma del Editor ===";
    spanish["current_language"] = "Idioma actual: ";
    spanish["select_language"] = "Seleccionar idioma:";
    spanish["back_to_main"] = "Volver al menú principal";
    spanish["language_changed"] = "?Idioma cambiado exitosamente!";  // 修正倒感叹号
    spanish["editor_settings"] = "=== Configuración del Editor ===";
    spanish["color_settings"] = "=== Configuración de Colores ===";
    spanish["current_background"] = "Color de fondo actual: ";
    spanish["current_code_color"] = "Color de código actual: ";
    spanish["current_comment_color"] = "Color de comentario actual: ";
    spanish["select_background_color"] = "Seleccionar color de fondo:";
    spanish["select_code_color"] = "Seleccionar color de código:";
    spanish["select_comment_color"] = "Seleccionar color de comentario:";
    spanish["color_options"] = "1. Colores predefinidos\n2. Color RGB personalizado\n3. Volver a configuración de colores";
    spanish["predefined_colors"] = "Colores predefinidos:";
    spanish["enter_rgb"] = "Ingrese valores RGB (0-255) separados por espacios (R G B): ";
    spanish["color_changed"] = "?Color cambiado exitosamente!";  // 修正倒感叹号
    spanish["invalid_rgb"] = "?Valores RGB inválidos! Usando color por defecto.";  // 修正倒感叹号
    spanish["reset_colors"] = "Restablecer colores por defecto";
    spanish["colors_reset"] = "?Colores restablecidos a los valores por defecto!";  // 修正倒感叹号
    translations[SPANISH] = spanish;
    
    // French (补全main_menu1-4和debug_program，调整序号)
    std::map<std::string, std::string> french;
    french["main_menu"] = "IDE Brainfuck";
    french["main_menu1"] = "Créer un Programme";  // 补全
    french["main_menu2"] = "Ouvrir un Programme";  // 补全
    french["main_menu3"] = "Paramètres de l'Editeur";  // 补全
    french["main_menu4"] = "Quitter";  // 补全
    french["editor_title"] = "=== éditeur de Programme Brainfuck ===";  // 修正首字母大写
    french["current_program"] = "Programme Actuel (Original):";
    french["filtered_code"] = "Code exécutable filtré:";
    french["editor_commands"] = "Commandes de l'éditeur:";
    french["edit_program"] = "1. Saisir/éditer le programme (supporte les commentaires)";
    french["run_program"] = "2. Exécuter le programme";
    french["debug_program"] = "3. Déboguer le programme";  // 补全
    french["save_program"] = "4. Sauvegarder le programme (avec commentaires)";  // 调整序号
    french["clear_program"] = "5. Effacer le programme";  // 调整序号
    french["show_filtered"] = "6. Afficher le code filtré";  // 调整序号
    french["language_settings_editor"] = "7. Paramètres de Langue";  // 调整序号
    french["back_menu"] = "8. Retour au menu principal";  // 调整序号
    french["select_option"] = "Sélectionner: ";
    french["program_empty"] = "Le programme est vide !";
    french["program_cleared"] = "Programme effacé !";
    french["invalid_choice"] = "Choix invalide !";
    french["enter_filename"] = "Entrez le nom du fichier (sans extension): ";
    french["save_success"] = "Programme sauvegardé avec succès !";
    french["save_failed"] = "échec de la sauvegarde !";  // 修正首字母大写
    french["run_results"] = "Résultats de l'Exécution:";
    french["run_success"] = "Programme exécuté avec succès.";
    french["pointer_error"] = "Pointeur hors limites !";
    french["compile_error"] = "Erreur de compilation !";
    french["input_program"] = "Entrez le programme Brainfuck (les caractères autres que les 8 commandes valides et //, /* */ sont traités comme des commentaires, entrez '0' seul pour terminar):";
    french["comments_supported"] = "Supporte les commentaires d'une ligne (//) et multi-lignes (/* */), les autres caractères sont également traités comme des commentaires";
    french["no_bf_files"] = "Aucun fichier .bf trouvé !";
    french["found_files"] = "Trouvé %d fichiers .bf (incluant les sous-répertoires):";
    french["open_file_prompt"] = "Entrez le numéro du fichier à ouvrir (0 pour revenir): ";
    french["invalid_selection"] = "Sélection invalide !";
    french["opening_file"] = "Ouverture du fichier: %s";
    french["file_content"] = "Contenu du programme (avec commentaires):";
    french["enter_to_editor"] = "Appuyez sur une touche pour entrer dans l'éditeur...";
    french["file_empty"] = "Le fichier est vide ou la lecture a échoué !";
    french["language_settings"] = "=== Paramètres de Langue de l'Editeur ===";
    french["current_language"] = "Langue actuelle: ";
    french["select_language"] = "Sélectionner la langue:";
    french["back_to_main"] = "Retour au menu principal";
    french["language_changed"] = "Langue changée avec succès !";
    french["editor_settings"] = "=== Paramètres de l'Editeur ===";
    french["color_settings"] = "=== Paramètres de Couleur ===";
    french["current_background"] = "Couleur de fond actuelle: ";
    french["current_code_color"] = "Couleur du code actuelle: ";
    french["current_comment_color"] = "Couleur des commentaires actuelle: ";
    french["select_background_color"] = "Sélectionner la couleur de fond:";
    french["select_code_color"] = "Sélectionner la couleur du code:";
    french["select_comment_color"] = "Sélectionner la couleur des commentaires:";
    french["color_options"] = "1. Couleurs prédéfinies\n2. Couleur RVB personnalisée\n3. Retour aux paramètres de couleur";
    french["predefined_colors"] = "Couleurs prédéfinies:";
    french["enter_rgb"] = "Entrez les valeurs RVB (0-255) séparées par des espaces (R V B): ";
    french["color_changed"] = "Couleur changée avec succès !";
    french["invalid_rgb"] = "Valeurs RVB invalides ! Utilisation de la couleur par défaut.";
    french["reset_colors"] = "Réinitialiser les couleurs par défaut";
    french["colors_reset"] = "Couleurs réinitialisées aux valeurs par défaut !";
    translations[FRENCH] = french;
    
    // German (修正特殊字符，补全main_menu1-4和debug_program，调整序号)
    std::map<std::string, std::string> german;
    german["main_menu"] = "Brainfuck IDE";
    german["main_menu1"] = "Programm erstellen";  // 补全
    german["main_menu2"] = "Programm ?ffnen";  // 补全（修正?）
    german["main_menu3"] = "Editor-Einstellungen";  // 补全
    german["main_menu4"] = "Beenden";  // 补全
    german["editor_title"] = "=== Brainfuck Programm-Editor ===";
    german["current_program"] = "Aktuelles Programm (Original):";
    german["filtered_code"] = "Gefilterter ausführbarer Code:";
    german["editor_commands"] = "Editor-Befehle:";
    german["edit_program"] = "1. Programm eingeben/bearbeiten (unterstützt Kommentare)";
    german["run_program"] = "2. Programm ausführen";
    german["debug_program"] = "3. Programm debuggen";  // 补全
    german["save_program"] = "4. Programm speichern (mit Kommentaren)";  // 调整序号
    german["clear_program"] = "5. Programm l?schen";  // 调整序号（修正?）
    german["show_filtered"] = "6. Gefilterten Code anzeigen";  // 调整序号
    german["language_settings_editor"] = "7. Spracheinstellungen";  // 调整序号
    german["back_menu"] = "8. Zurück zum Hauptmenü";  // 调整序号
    german["select_option"] = "Auswahl: ";
    german["program_empty"] = "Programm ist leer!";
    german["program_cleared"] = "Programm gel?scht!";  // 修正?
    german["invalid_choice"] = "Ungültige Auswahl!";
    german["enter_filename"] = "Dateinamen eingeben (ohne Erweiterung): ";
    german["save_success"] = "Programm erfolgreich gespeichert!";
    german["save_failed"] = "Speichern fehlgeschlagen!";
    german["run_results"] = "Ausführungsergebnisse:";
    german["run_success"] = "Programm erfolgreich ausgeführt.";
    german["pointer_error"] = "Zeiger au?erhalb der Grenzen!";  // 修正?
    german["compile_error"] = "Kompilierungsfehler!";
    german["input_program"] = "Brainfuck-Programm eingeben (Zeichen au?er den 8 gültigen Befehlen und //, /* */ werden als Kommentare behandelt, '0' alleine eingeben zum Beenden):";
    german["comments_supported"] = "Unterstützt einzeilige (//) und mehrzeilige (/* */) Kommentare, andere Zeichen werden ebenfalls als Kommentare behandelt";
    german["no_bf_files"] = "Keine .bf-Dateien gefunden!";
    german["found_files"] = "%d .bf-Dateien gefunden (einschlie?lich Unterverzeichnisse):";  // 修正?
    german["open_file_prompt"] = "Dateinummer zum ?ffnen eingeben (0 zum Zurückkehren): ";  // 修正?
    german["invalid_selection"] = "Ungültige Auswahl!";
    german["opening_file"] = "?ffne Datei: %s";  // 修正?
    german["file_content"] = "Programminhalt (mit Kommentaren):";
    german["enter_to_editor"] = "Drücken Sie eine beliebige Taste, um den Editor zu ?ffnen...";  // 修正?
    german["file_empty"] = "Datei ist leer oder Lesen fehlgeschlagen!";
    german["language_settings"] = "=== Editor-Spracheinstellungen ===";
    german["current_language"] = "Aktuelle Sprache: ";
    german["select_language"] = "Sprache ausw?hlen:";  // 修正?
    german["back_to_main"] = "Zurück zum Hauptmenü";
    german["language_changed"] = "Sprache erfolgreich ge?ndert!";  // 修正?
    german["editor_settings"] = "=== Editor-Einstellungen ===";
    german["color_settings"] = "=== Farbeinstellungen ===";
    german["current_background"] = "Aktuelle Hintergrundfarbe: ";
    german["current_code_color"] = "Aktuelle Codefarbe: ";
    german["current_comment_color"] = "Aktuelle Kommentarfarbe: ";
    german["select_background_color"] = "Hintergrundfarbe ausw?hlen:";  // 修正?
    german["select_code_color"] = "Codefarbe ausw?hlen:";  // 修正?
    german["select_comment_color"] = "Kommentarfarbe ausw?hlen:";  // 修正?
    german["color_options"] = "1. Vordefinierte Farben\n2. Benutzerdefinierte RGB-Farbe\n3. Zurück zu Farbeinstellungen";
    german["predefined_colors"] = "Vordefinierte Farben:";
    german["enter_rgb"] = "RGB-Werte eingeben (0-255) durch Leerzeichen getrennt (R G B): ";
    german["color_changed"] = "Farbe erfolgreich ge?ndert!";  // 修正?
    german["invalid_rgb"] = "Ungültige RGB-Werte! Verwende Standardfarbe.";
    german["reset_colors"] = "Auf Standardfarben zurücksetzen";
    german["colors_reset"] = "Farben auf Standardwerte zurückgesetzt!";
    translations[GERMAN] = german;
    
    // Russian (补全main_menu1-4和debug_program，调整序号)
    std::map<std::string, std::string> russian;
    russian["main_menu"] = "Brainfuck IDE";
    russian["main_menu1"] = "Создать программу";  // 补全
    russian["main_menu2"] = "Открыть программу";  // 补全
    russian["main_menu3"] = "Настройки редактора";  // 补全
    russian["main_menu4"] = "Выйти";  // 补全
    russian["editor_title"] = "=== Редактор программ Brainfuck ===";
    russian["current_program"] = "Текущая программа (оригинал):";
    russian["filtered_code"] = "Отфильтрованный исполняемый код:";
    russian["editor_commands"] = "Команды редактора:";
    russian["edit_program"] = "1. Ввод/Редактирование программы (поддерживает комментарии)";
    russian["run_program"] = "2. Запуск программы";
    russian["debug_program"] = "3. Отладить программу";  // 补全
    russian["save_program"] = "4. Сохранить программу (с комментариями)";  // 调整序号
    russian["clear_program"] = "5. Очистить программу";  // 调整序号
    russian["show_filtered"] = "6. Показать отфильтрованный код";  // 调整序号
    russian["language_settings_editor"] = "7. Настройки языка";  // 调整序号
    russian["back_menu"] = "8. Вернуться в главное меню";  // 调整序号
    russian["select_option"] = "Выбор: ";
    russian["program_empty"] = "Программа пуста!";
    russian["program_cleared"] = "Программа очищена!";
    russian["invalid_choice"] = "Неверный выбор!";
    russian["enter_filename"] = "Введите имя файла (без расширения): ";
    russian["save_success"] = "Программа успешно сохранена!";
    russian["save_failed"] = "Ошибка сохранения!";
    russian["run_results"] = "Результаты выполнения:";
    russian["run_success"] = "Программа выполнена успешно.";
    russian["pointer_error"] = "Указатель вне допустимого диапазона!";
    russian["compile_error"] = "Ошибка компиляции!";
    russian["input_program"] = "Введите программу Brainfuck (символы, отличные от 8 допустимых команд и //, /* */, рассматриваются как комментарии, введите '0' отдельно для завершения):";
    russian["comments_supported"] = "Поддерживает однострочные (//) и многострочные (/* */) комментарии, другие символы также рассматриваются как комментарии";
    russian["no_bf_files"] = "Файлы .bf не найдены!";
    russian["found_files"] = "Найдено %d файлов .bf (включая подкаталоги):";
    russian["open_file_prompt"] = "Введите номер файла для открытия (0 для возврата): ";
    russian["invalid_selection"] = "Неверный выбор!";
    russian["opening_file"] = "Открытие файла: %s";
    russian["file_content"] = "Содержимое программы (с комментариями):";
    russian["enter_to_editor"] = "Нажмите любую клавишу для входа в редактор...";
    russian["file_empty"] = "Файл пуст или чтение не удалось!";
    russian["language_settings"] = "=== Настройки языка редактора ===";
    russian["current_language"] = "Текущий язык: ";
    russian["select_language"] = "Выберите язык:";
    russian["back_to_main"] = "Вернуться в главное меню";
    russian["language_changed"] = "Язык успешно изменен!";
    russian["editor_settings"] = "=== Настройки редактора ===";
    russian["color_settings"] = "=== Настройки цвета ===";
    russian["current_background"] = "Текущий цвет фона: ";
    russian["current_code_color"] = "Текущий цвет кода: ";
    russian["current_comment_color"] = "Текущий цвет комментариев: ";
    russian["select_background_color"] = "Выберите цвет фона:";
    russian["select_code_color"] = "Выберите цвет кода:";
    russian["select_comment_color"] = "Выберите цвет комментариев:";
    russian["color_options"] = "1. Предопределенные цвета\n2. Пользовательский RGB цвет\n3. Вернуться к настройкам цвета";
    russian["predefined_colors"] = "Предопределенные цвета:";
    russian["enter_rgb"] = "Введите значения RGB (0-255) через пробел (R G B): ";
    russian["color_changed"] = "Цвет успешно изменен!";
    russian["invalid_rgb"] = "Неверные значения RGB! Используется цвет по умолчанию.";
    russian["reset_colors"] = "Сбросить цвета по умолчанию";
    russian["colors_reset"] = "Цвета сброшены к значениям по умолчанию!";
    translations[RUSSIAN] = russian;
    
    // Portuguese (修正特殊字符，补全main_menu1-4和debug_program，调整序号)
    std::map<std::string, std::string> portuguese;
    portuguese["main_menu"] = "IDE Brainfuck";
    portuguese["main_menu1"] = "Criar Programa";  // 补全
    portuguese["main_menu2"] = "Abrir Programa";  // 补全
    portuguese["main_menu3"] = "Configura??es do Editor";  // 补全（修正??）
    portuguese["main_menu4"] = "Sair";  // 补全
    portuguese["editor_title"] = "=== Editor de Programa Brainfuck ===";
    portuguese["current_program"] = "Programa Atual (Original):";
    portuguese["filtered_code"] = "Código executável filtrado:";
    portuguese["editor_commands"] = "Comandos do Editor:";
    portuguese["edit_program"] = "1. Inserir/Editar programa (suporta comentários)";
    portuguese["run_program"] = "2. Executar programa";
    portuguese["debug_program"] = "3. Depurar programa";  // 补全
    portuguese["save_program"] = "4. Salvar programa (com comentários)";  // 调整序号
    portuguese["clear_program"] = "5. Limpar programa";  // 调整序号
    portuguese["show_filtered"] = "6. Mostrar código filtrado";  // 调整序号
    portuguese["language_settings_editor"] = "7. Configura??es de Idioma";  // 调整序号（修正??）
    portuguese["back_menu"] = "8. Voltar ao menu principal";  // 调整序号
    portuguese["select_option"] = "Selecionar: ";
    portuguese["program_empty"] = "Programa está vazio!";
    portuguese["program_cleared"] = "Programa limpo!";
    portuguese["invalid_choice"] = "Escolha inválida!";
    portuguese["enter_filename"] = "Digite o nome do arquivo (sem extens?o): ";  // 修正?o
    portuguese["save_success"] = "Programa salvo com sucesso!";
    portuguese["save_failed"] = "Falha ao salvar!";
    portuguese["run_results"] = "Resultados da Execu??o:";  // 修正??o
    portuguese["run_success"] = "Programa executado com sucesso.";
    portuguese["pointer_error"] = "Ponteiro fora dos limites!";
    portuguese["compile_error"] = "Erro de compila??o!";  // 修正??o
    portuguese["input_program"] = "Digite o programa Brainfuck (caracteres diferentes dos 8 comandos válidos e //, /* */ s?o tratados como comentários, digite '0' sozinho para terminar):";
    portuguese["comments_supported"] = "Suporta comentários de linha única (//) e multi-linha (/* */), outros caracteres também s?o tratados como comentários";
    portuguese["no_bf_files"] = "Nenhum arquivo .bf encontrado!";
    portuguese["found_files"] = "Encontrados %d arquivos .bf (incluindo subdiretórios):";
    portuguese["open_file_prompt"] = "Digite o número do arquivo para abrir (0 para voltar): ";
    portuguese["invalid_selection"] = "Sele??o inválida!";  // 修正??o
    portuguese["opening_file"] = "Abrindo arquivo: %s";
    portuguese["file_content"] = "Conteúdo do programa (com comentários):";
    portuguese["enter_to_editor"] = "Pressione qualquer tecla para entrar no editor...";
    portuguese["file_empty"] = "Arquivo está vazio ou a leitura falhou!";
    portuguese["language_settings"] = "=== Configura??es de Idioma do Editor ===";  // 修正??
    portuguese["current_language"] = "Idioma atual: ";
    portuguese["select_language"] = "Selecionar idioma:";
    portuguese["back_to_main"] = "Voltar ao menu principal";
    portuguese["language_changed"] = "Idioma alterado com sucesso!";
    portuguese["editor_settings"] = "=== Configura??es do Editor ===";  // 修正??
    portuguese["color_settings"] = "=== Configura??es de Cor ===";  // 修正??
    portuguese["current_background"] = "Cor de fundo atual: ";
    portuguese["current_code_color"] = "Cor do código atual: ";
    portuguese["current_comment_color"] = "Cor do comentário atual: ";
    portuguese["select_background_color"] = "Selecionar cor de fundo:";
    portuguese["select_code_color"] = "Selecionar cor do código:";
    portuguese["select_comment_color"] = "Selecionar cor do comentário:";
    portuguese["color_options"] = "1. Cores predefinidas\n2. Cor RGB personalizada\n3. Voltar às configura??es de cor";  // 修正??
    portuguese["predefined_colors"] = "Cores predefinidas:";
    portuguese["enter_rgb"] = "Digite os valores RGB (0-255) separados por espa?os (R G B): ";
    portuguese["color_changed"] = "Cor alterada com sucesso!";
    portuguese["invalid_rgb"] = "Valores RGB inválidos! Usando cor padr?o.";  // 修正?o
    portuguese["reset_colors"] = "Redefinir cores padr?o";  // 修正?o
    portuguese["colors_reset"] = "Cores redefinidas para os valores padr?o!";  // 修正?o
    translations[PORTUGUESE] = portuguese;
}

// 语言名称映射
std::map<Language, std::string> languageNames;

// 初始化语言名称
void initializeLanguageNames() {
    languageNames[ENGLISH] = "English";
    languageNames[CHINESE] = "中文 (Chinese)";
    languageNames[SPANISH] = "Espa?ol (Spanish)";
    languageNames[FRENCH] = "Fran?ais (French)";
    languageNames[GERMAN] = "Deutsch (German)";
    languageNames[JAPANESE] = "日本語 (Japanese)";
    languageNames[RUSSIAN] = "Русский (Russian)";
    languageNames[PORTUGUESE] = "Português (Portuguese)";
}

// 颜色名称映射
std::map<Color, std::string> colorNames;

// 初始化颜色名称
void initializeColorNames() {
    colorNames[BLACK] = "Black";
    colorNames[RED] = "Red";
    colorNames[GREEN] = "Green";
    colorNames[YELLOW] = "Yellow";
    colorNames[BLUE] = "Blue";
    colorNames[MAGENTA] = "Magenta";
    colorNames[CYAN] = "Cyan";
    colorNames[WHITE] = "White";
    colorNames[ORANGE] = "Orange";
    colorNames[BROWN] = "Brown";
}

// 颜色到控制台代码的映射
#ifdef _WIN32
std::map<Color, int> colorToWinConsole;
#else
std::map<Color, std::string> colorToLinuxConsole;
#endif

// 初始化颜色映射
void initializeColorMaps() {
#ifdef _WIN32
    colorToWinConsole[BLACK] = 0;
    colorToWinConsole[RED] = 4;
    colorToWinConsole[GREEN] = 2;
    colorToWinConsole[YELLOW] = 6;
    colorToWinConsole[BLUE] = 1;
    colorToWinConsole[MAGENTA] = 5;
    colorToWinConsole[CYAN] = 3;
    colorToWinConsole[WHITE] = 7;
    colorToWinConsole[ORANGE] = 6;
    colorToWinConsole[BROWN] = 6;
#else
    colorToLinuxConsole[BLACK] = "30";
    colorToLinuxConsole[RED] = "31";
    colorToLinuxConsole[GREEN] = "32";
    colorToLinuxConsole[YELLOW] = "33";
    colorToLinuxConsole[BLUE] = "34";
    colorToLinuxConsole[MAGENTA] = "35";
    colorToLinuxConsole[CYAN] = "36";
    colorToLinuxConsole[WHITE] = "37";
    colorToLinuxConsole[ORANGE] = "33";
    colorToLinuxConsole[BROWN] = "33";
#endif
}

class WinConsoleMenu {
private:
    std::vector<std::string> options;
    std::vector<std::function<void()>> actions;
    std::string title;
    int selectedIndex;
    HANDLE hConsole;
    COORD startPos;

    void setCursorPosition(int x, int y) {
        COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(hConsole, coord);
    }

    void setColor(WORD color) {
        SetConsoleTextAttribute(hConsole, color);
    }

    void clearScreen() {
        system("cls");
    }

    void displayMenu() {
        clearScreen();
        
        // 显示标题
        setColor(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
        std::cout << title << std::endl;
        
        // 显示选项
        for (size_t i = 0; i < options.size(); i++) {
            setCursorPosition(0, static_cast<int>(i) + 2);
            
            if (i == selectedIndex) {
                setColor(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
                std::cout << "> " << options[i];
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                std::cout << "  " << options[i];
            }
        }
        
        setCursorPosition(0, static_cast<int>(options.size()) + 3);
        std::cout << "-----------------------------------\n";
    }

    bool processKeyboardEvent() {
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        DWORD numEvents;
        INPUT_RECORD inputRecord;

        while (true) {
            ReadConsoleInput(hInput, &inputRecord, 1, &numEvents);
            
            if (inputRecord.EventType == KEY_EVENT) {
                KEY_EVENT_RECORD& ker = inputRecord.Event.KeyEvent;
                
                if (ker.bKeyDown) {
                    switch (ker.wVirtualKeyCode) {
                        case VK_UP:
                            selectedIndex = (selectedIndex - 1 + static_cast<int>(options.size())) % options.size();
                            displayMenu();
                            break;
                            
                        case VK_DOWN:
                            selectedIndex = (selectedIndex + 1) % options.size();
                            displayMenu();
                            break;
                            
                        case VK_RETURN:
                            return true; // 确认选择
                            
                        case VK_ESCAPE:
                            selectedIndex = -1;
                            return true; // 退出
                    }
                }
            }
        }
        return false;
    }

public:
    WinConsoleMenu(const std::string& menuTitle) 
        : title(menuTitle), selectedIndex(0) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // 保存初始光标位置
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        startPos = csbi.dwCursorPosition;
    }

    void addOption(const std::string& option, std::function<void()> action = nullptr) {
        options.push_back(option);
        actions.push_back(action);
    }

    int show() {
        if (options.empty()) {
            return -1;
        }

        selectedIndex = 0;
        displayMenu();

        return processKeyboardEvent() ? selectedIndex : -1;
    }

    void executeSelected() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(actions.size()) && actions[selectedIndex]) {
            actions[selectedIndex]();
        }
    }

    ~WinConsoleMenu() {
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
};

// 等待按键的函数
void waitForKey() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD numEvents;
    INPUT_RECORD inputRecord;
    
    while (true) {
        ReadConsoleInput(hInput, &inputRecord, 1, &numEvents);
        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
            break;
        }
    }
}



// 全局语言设置 - 默认英文
Language currentLanguage = ENGLISH;

// 全局颜色设置
Color backgroundColor = BLACK;
Color codeColor = WHITE;
Color commentColor = GREEN;

// 函数声明
void languageSettings();
void colorSettings();
void saveSettings();
void loadSettings();
void applyColors();
void applyCommentColor();
void resetColors();
std::string getExeDir();
bool createDirectory(const std::string& path);
void clearScreen();
void pauseScreen();

// 获取翻译文本
std::string tr(const std::string& key) {
    std::map<Language, std::map<std::string, std::string> >::iterator langIt = translations.find(currentLanguage);
    if (langIt != translations.end()) {
        std::map<std::string, std::string>::iterator it = langIt->second.find(key);
        if (it != langIt->second.end()) {
            return it->second;
        }
    }
    // 如果当前语言没有翻译，回退到英语
    if (currentLanguage != ENGLISH) {
        std::map<Language, std::map<std::string, std::string> >::iterator engIt = translations.find(ENGLISH);
        if (engIt != translations.end()) {
            std::map<std::string, std::string>::iterator engKeyIt = engIt->second.find(key);
            if (engKeyIt != engIt->second.end()) {
                return engKeyIt->second;
            }
        }
    }
    return key; // 如果英语也没有，返回键名
}

// 格式化字符串（支持 %d 等）
std::string trf(const std::string& key, int value) {
    std::string text = tr(key);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), text.c_str(), value);
    return std::string(buffer);
}

std::string trf(const std::string& key, const char* value) {
    std::string text = tr(key);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), text.c_str(), value);
    return std::string(buffer);
}

// 应用颜色设置到控制台
void applyColors() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int bgColor = colorToWinConsole[backgroundColor];
    int fgColor = colorToWinConsole[codeColor];
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | fgColor);
#else
    // Linux终端颜色设置
    std::string bgCode = colorToLinuxConsole[backgroundColor];
    std::string fgCode = colorToLinuxConsole[codeColor];
    std::cout << "\033[" << bgCode << "m\033[" << fgCode << "m";
#endif
}

// 应用注释颜色
void applyCommentColor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int bgColor = colorToWinConsole[backgroundColor];
    int fgColor = colorToWinConsole[commentColor];
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | fgColor);
#else
    // Linux终端颜色设置
    std::string bgCode = colorToLinuxConsole[backgroundColor];
    std::string fgCode = colorToLinuxConsole[commentColor];
    std::cout << "\033[" << bgCode << "m\033[" << fgCode << "m";
#endif
}

// 重置控制台颜色
void resetColors() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // 默认颜色：黑底白字
#else
    std::cout << "\033[0m";
#endif
}

// 保存设置到文件
void saveSettings() {
    std::string exeDir = getExeDir();
    std::string filePath;
    
#ifdef _WIN32
    filePath = exeDir + "\\editor.txt";
#else
    filePath = exeDir + "/editor.txt";
#endif
    
    std::ofstream file(filePath.c_str());
    if (file.is_open()) {
        // 保存语言设置
        file << "language:" << static_cast<int>(currentLanguage) << std::endl;
        
        // 保存颜色设置
        file << "color:" << colorNames[backgroundColor] << ";" 
             << colorNames[codeColor] << ";" 
             << colorNames[commentColor] << std::endl;
        
        file.close();
    } else {
        std::cerr << "Failed to save settings to: " << filePath << std::endl;
    }
}

// 从文件加载设置
void loadSettings() {
    std::string exeDir = getExeDir();
    std::string filePath;
    
#ifdef _WIN32
    filePath = exeDir + "\\editor.txt";
#else
    filePath = exeDir + "/editor.txt";
#endif
    
    std::ifstream file(filePath.c_str());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("language:") == 0) {
                int langValue = atoi(line.substr(9).c_str());
                if (langValue >= 0 && langValue <= 7) {
                    currentLanguage = static_cast<Language>(langValue);
                }
            } else if (line.find("color:") == 0) {
                std::string colorStr = line.substr(6);
                std::istringstream ss(colorStr);
                std::string bgColor, codeColorStr, commentColorStr;
                
                if (std::getline(ss, bgColor, ';') &&
                    std::getline(ss, codeColorStr, ';') &&
                    std::getline(ss, commentColorStr, ';')) {
                    
                    // 查找背景颜色
                    for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
                        if (it->second == bgColor) {
                            backgroundColor = it->first;
                            break;
                        }
                    }
                    
                    // 查找代码颜色
                    for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
                        if (it->second == codeColorStr) {
                            codeColor = it->first;
                            break;
                        }
                    }
                    
                    // 查找注释颜色
                    for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
                        if (it->second == commentColorStr) {
                            commentColor = it->first;
                            break;
                        }
                    }
                }
            }
        }
        file.close();
    }
    
    // 应用加载的颜色设置
    applyColors();
}

// 语言设置菜单
void languageSettings() {
    WinConsoleMenu langMenu(tr("language_settings"));
    
    // 添加语言选项
    for (std::map<Language, std::string>::iterator it = languageNames.begin(); it != languageNames.end(); ++it) {
        langMenu.addOption(it->second, [it]() {
            currentLanguage = it->first;
            std::cout << tr("language_changed") << std::endl;
            saveSettings();
            pauseScreen();
        });
    }
    
    // 添加返回选项
    langMenu.addOption(tr("back_to_main"), []() {});
    
    while (true) {
        int choice = langMenu.show();
        if (choice == -1 || choice == static_cast<int>(languageNames.size())) {
            break; // ESC或选择返回
        }
        langMenu.executeSelected();
    }
}

// 颜色设置菜单
void colorSettings() {
    WinConsoleMenu colorMenu(tr("color_settings"));
    
    colorMenu.addOption(tr("select_background_color"), []() {
        WinConsoleMenu bgColorMenu(tr("select_background_color"));
        
        for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
            bgColorMenu.addOption(it->second, [it]() {
                backgroundColor = it->first;
                std::cout << tr("color_changed") << std::endl;
                applyColors();
                saveSettings();
                pauseScreen();
            });
        }
        bgColorMenu.addOption(tr("back_to_main"), []() {});
        
        while (true) {
            int choice = bgColorMenu.show();
            if (choice == -1 || choice == static_cast<int>(colorNames.size())) {
                break;
            }
            bgColorMenu.executeSelected();
        }
    });
    
    colorMenu.addOption(tr("select_code_color"), []() {
        WinConsoleMenu codeColorMenu(tr("select_code_color"));
        
        for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
            codeColorMenu.addOption(it->second, [it]() {
                codeColor = it->first;
                std::cout << tr("color_changed") << std::endl;
                applyColors();
                saveSettings();
                pauseScreen();
            });
        }
        codeColorMenu.addOption(tr("back_to_main"), []() {});
        
        while (true) {
            int choice = codeColorMenu.show();
            if (choice == -1 || choice == static_cast<int>(colorNames.size())) {
                break;
            }
            codeColorMenu.executeSelected();
        }
    });
    
    colorMenu.addOption(tr("select_comment_color"), []() {
        WinConsoleMenu commentColorMenu(tr("select_comment_color"));
        
        for (std::map<Color, std::string>::iterator it = colorNames.begin(); it != colorNames.end(); ++it) {
            commentColorMenu.addOption(it->second, [it]() {
                commentColor = it->first;
                std::cout << tr("color_changed") << std::endl;
                saveSettings();
                pauseScreen();
            });
        }
        commentColorMenu.addOption(tr("back_to_main"), []() {});
        
        while (true) {
            int choice = commentColorMenu.show();
            if (choice == -1 || choice == static_cast<int>(colorNames.size())) {
                break;
            }
            commentColorMenu.executeSelected();
        }
    });
    
    colorMenu.addOption(tr("reset_colors"), []() {
        backgroundColor = BLACK;
        codeColor = WHITE;
        commentColor = GREEN;
        std::cout << tr("colors_reset") << std::endl;
        applyColors();
        saveSettings();
        pauseScreen();
    });
    
    colorMenu.addOption(tr("back_to_main"), []() {});
    
    while (true) {
        int choice = colorMenu.show();
        if (choice == -1 || choice == 4) { // 第5个选项是返回
            break;
        }
        colorMenu.executeSelected();
    }
}

// 编辑器设置菜单
void editorSettings() {
    WinConsoleMenu settingsMenu(tr("editor_settings"));
    
    settingsMenu.addOption(tr("language_settings"), languageSettings);
    settingsMenu.addOption(tr("color_settings"), colorSettings);
    settingsMenu.addOption(tr("back_to_main"), []() {});
    
    while (true) {
        int choice = settingsMenu.show();
        if (choice == -1 || choice == 2) {
            break;
        }
        settingsMenu.executeSelected();
    }
}

class DirectoryReader {
public:
    // 查找以 .bf 结尾的文件
    static std::vector<std::string> getBFFiles(const std::string& directoryPath) {
        std::vector<std::string> files;
        
#ifdef _WIN32
        // Windows 实现
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((directoryPath + "\\*").c_str(), &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            std::cerr << "无法打开目录: " << directoryPath << std::endl;
            return files;
        }
        do {
            // 跳过 "." 和 ".."
            if (strcmp(findFileData.cFileName, ".") == 0 || 
                strcmp(findFileData.cFileName, "..") == 0) {
                continue;
            }
            // 如果是文件而不是目录，并且以 .bf 结尾
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filename = findFileData.cFileName;
                if (hasBFExtension(filename)) {
                    files.push_back(filename);
                }
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);
        FindClose(hFind);
#else
        // Linux 实现
        DIR* dir = opendir(directoryPath.c_str());
        if (dir == NULL) {
            std::cerr << "无法打开目录: " << directoryPath << std::endl;
            return files;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            // 跳过 "." 和 ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            if (entry->d_type == DT_REG) {
                std::string filename = entry->d_name;
                if (hasBFExtension(filename)) {
                    files.push_back(filename);
                }
            }
        }
        closedir(dir);
#endif
        return files;
    }

    // 获取所有 .bf 文件的完整路径
    static std::vector<std::string> getBFFilesWithPath(const std::string& directoryPath) {
        std::vector<std::string> files;
        
#ifdef _WIN32
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((directoryPath + "\\*").c_str(), &findFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findFileData.cFileName, ".") != 0 && 
                    strcmp(findFileData.cFileName, "..") != 0) {
                    if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        std::string filename = findFileData.cFileName;
                        if (hasBFExtension(filename)) {
                            std::string fullPath = directoryPath + "\\" + filename;
                            files.push_back(fullPath);
                        }
                    }
                }
            } while (FindNextFileA(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(directoryPath.c_str());
        if (dir != NULL) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    if (entry->d_type == DT_REG) {
                        std::string filename = entry->d_name;
                        if (hasBFExtension(filename)) {
                            std::string fullPath = directoryPath + "/" + filename;
                            files.push_back(fullPath);
                        }
                    }
                }
            }
            closedir(dir);
        }
#endif
        return files;
    }

    // 递归查找所有子目录中的 .bf 文件
    static std::vector<std::string> getBFFilesRecursive(const std::string& directoryPath) {
        std::vector<std::string> allBFFiles;
        // 先获取当前目录的 .bf 文件
        std::vector<std::string> currentDirFiles = getBFFilesWithPath(directoryPath);
        for (std::vector<std::string>::size_type i = 0; i < currentDirFiles.size(); i++) {
            allBFFiles.push_back(currentDirFiles[i]);
        }
        
#ifdef _WIN32
        // 递归查找子目录 - Windows
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA((directoryPath + "\\*").c_str(), &findFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findFileData.cFileName, ".") != 0 && 
                    strcmp(findFileData.cFileName, "..") != 0) {
                    
                    if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        std::string subDirPath = directoryPath + "\\" + findFileData.cFileName;
                        std::vector<std::string> subDirFiles = getBFFilesRecursive(subDirPath);
                        for (std::vector<std::string>::size_type i = 0; i < subDirFiles.size(); i++) {
                            allBFFiles.push_back(subDirFiles[i]);
                        }
                    }
                }
            } while (FindNextFileA(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
#else
        // 递归查找子目录 - Linux
        DIR* dir = opendir(directoryPath.c_str());
        if (dir != NULL) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    if (entry->d_type == DT_DIR) {
                        std::string subDirPath = directoryPath + "/" + entry->d_name;
                        std::vector<std::string> subDirFiles = getBFFilesRecursive(subDirPath);
                        for (std::vector<std::string>::size_type i = 0; i < subDirFiles.size(); i++) {
                            allBFFiles.push_back(subDirFiles[i]);
                        }
                    }
                }
            }
            closedir(dir);
        }
#endif
        return allBFFiles;
    }

private:
    // 检查文件是否以 .bf 结尾（不区分大小写）
    static bool hasBFExtension(const std::string& filename) {
        if (filename.length() < 3) {
            return false;
        }
        std::string extension = filename.substr(filename.length() - 3);
        // 不区分大小写比较
        return (extension == ".bf" || extension == ".BF" || 
                extension == ".Bf" || extension == ".bF");
    }
};

std::string getExeDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        std::string fullPath(path);
        std::string::size_type pos = fullPath.find_last_of("\\");
        if (pos != std::string::npos) {
            return fullPath.substr(0, pos);
        }
    }
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        std::string fullPath(path, count);
        std::string::size_type pos = fullPath.find_last_of("/");
        if (pos != std::string::npos) {
            return fullPath.substr(0, pos);
        }
    }
#endif
    return "";
}

// 创建目录
bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) != 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

// 清屏函数
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 暂停函数
void pauseScreen() {
#ifdef _WIN32
    system("pause");
#else
    printf("按任意键继续...");
    getchar();
    getchar();
#endif
}

int find_all_bf(std::string path) {
    // 使用递归搜索
    std::vector<std::string> bfFiles = DirectoryReader::getBFFilesRecursive(path);
    std::cout << trf("found_files", bfFiles.size()) << std::endl;
    for (std::vector<std::string>::size_type i = 0; i < bfFiles.size(); i++) {
        // 显示相对路径
        std::string relativePath = bfFiles[i].substr(path.length() + 1);
        std::cout << i + 1 << ". " << relativePath << std::endl;
    }
    return bfFiles.size();
}

// 修改：返回原始输入和过滤后的程序
struct ProgramData {
    std::string original;  // 原始输入（包含注释）
    std::string filtered;  // 过滤后的程序（只包含有效指令）
};

// 显示带颜色的程序内容
void displayProgramWithColors(const std::string& program) {
    bool inMultiLineComment = false;
    bool inSingleLineComment = false;
    
    for (std::string::size_type i = 0; i < program.length(); i++) {
        char ch = program[i];
        char next_ch = (i + 1 < program.length()) ? program[i + 1] : '\0';
        
        // 处理多行注释
        if (!inSingleLineComment && ch == '/' && next_ch == '*') {
            inMultiLineComment = true;
            applyCommentColor();
            printf("%c", ch);
            i++; // 跳过下一个字符
            if (i < program.length()) {
                printf("%c", program[i]);
            }
            continue;
        }
        if (inMultiLineComment && ch == '*' && next_ch == '/') {
            inMultiLineComment = false;
            applyCommentColor();
            printf("%c", ch);
            i++; // 跳过下一个字符
            if (i < program.length()) {
                printf("%c", program[i]);
            }
            applyColors(); // 恢复代码颜色
            continue;
        }
        if (inMultiLineComment) {
            applyCommentColor();
            printf("%c", ch);
            continue;
        }
        
        // 处理单行注释
        if (!inMultiLineComment && ch == '/' && next_ch == '/') {
            inSingleLineComment = true;
            applyCommentColor();
            printf("%c", ch);
            i++; // 跳过下一个字符
            if (i < program.length()) {
                printf("%c", program[i]);
            }
            continue;
        }
        if (inSingleLineComment && ch == '\n') {
            inSingleLineComment = false;
            applyColors(); // 恢复代码颜色
            printf("%c", ch);
            continue;
        }
        if (inSingleLineComment) {
            applyCommentColor();
            printf("%c", ch);
            continue;
        }
        
        // 检查是否是有效Brainfuck指令
        if (ch == '[' || ch == ']' || ch == '<' || ch == '>' || 
            ch == '.' || ch == ',' || ch == '+' || ch == '-') {
            applyColors(); // 使用代码颜色
            printf("%c", ch);
        } else {
            // 其他字符视为注释
            applyCommentColor();
            printf("%c", ch);
        }
    }
    applyColors(); // 确保最后恢复代码颜色
}

// 辅助函数：移动控制台光标
void moveCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setConsoleColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 重置控制台颜色为默认
void resetConsoleColor() {
    setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// 高亮显示一行代码中的注释
void printLineWithHighlight(const std::string& line, int lineNumber) {
    // 保存当前颜色
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    WORD originalColor = info.wAttributes;

    // 打印行号
    printf("%2d: ", lineNumber);

    bool inMultiLineComment = false;
    bool inSingleLineComment = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];
        char next_ch = (i + 1 < line.length()) ? line[i + 1] : '\0';

        // 处理多行注释
        if (!inSingleLineComment && ch == '/' && next_ch == '*') {
            inMultiLineComment = true;
            setConsoleColor(FOREGROUND_GREEN); // 注释颜色
            printf("/*");
            i++;
            continue;
        }
        if (inMultiLineComment && ch == '*' && next_ch == '/') {
            printf("*/");
            i++;
            inMultiLineComment = false;
            setConsoleColor(originalColor); // 恢复原颜色
            continue;
        }
        
        // 处理单行注释
        if (!inMultiLineComment && ch == '/' && next_ch == '/') {
            inSingleLineComment = true;
            setConsoleColor(FOREGROUND_GREEN); // 注释颜色
            printf("//");
            i++;
            continue;
        }
        
        // 注释结束
        if (inSingleLineComment && ch == '\n') {
            inSingleLineComment = false;
            setConsoleColor(originalColor);
        }
        
        // 设置颜色
        if (inMultiLineComment || inSingleLineComment) {
            setConsoleColor(FOREGROUND_GREEN); // 注释颜色
        } else {
            setConsoleColor(originalColor); // 普通代码颜色
        }
        
        // 打印字符
        printf("%c", ch);
    }
    
    // 恢复原颜色
    setConsoleColor(originalColor);
    printf("\n");
}

ProgramData input_Bf(const std::string& initialData) {
    ProgramData data;
    
    printf("%s\n", tr("input_program").c_str());
    printf("%s\n", tr("comments_supported").c_str());
    printf("Brainfuck IDE(Press Ctrl + Enter to finish input)\n");
    printf("=====================================================================\n");
    
    // 保存原始控制台模式
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode;
    GetConsoleMode(hStdin, &originalMode);
    SetConsoleMode(hStdin, originalMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
    
    std::vector<std::string> lines;
    
    // 将初始数据分割成行
    if (!initialData.empty()) {
        std::stringstream ss(initialData);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
    }
    
    // 如果没有初始数据或初始数据为空，添加一个空行
    if (lines.empty()) {
        lines.push_back("");
    }
    
    int currentLine = 0;
    int cursorPos = 0;
    bool inputComplete = false;
    
    // 清空输入缓冲区
    while (_kbhit()) _getch();
    
    // 显示初始状态
    system("cls");
    printf("Brainfuck IDE(Press Ctrl + Enter to finish input)\n");
    printf("=====================================================================\n");
    
    // 显示所有行
    for (int i = 0; i < lines.size(); i++) {
        printLineWithHighlight(lines[i], i + 1);
    }
    printf("=====================================================================\n");
    printf("Current row: %d, Position: %d", currentLine + 1, cursorPos + 1);
    
    // 初始光标位置
    moveCursor(4 + cursorPos, 2 + currentLine);
    
    while (!inputComplete) {
        if (_kbhit()) {
            int ch = _getch();
            
            // 处理功能键
            if (ch == 0 || ch == 0xE0) {
                int extCh = _getch(); // 获取扩展键码
                
                // 方向键支持
                switch (extCh) {
                    case 72: // 上箭头
                        if (currentLine > 0) {
                            currentLine--;
                            if (cursorPos > lines[currentLine].length()) {
                                cursorPos = lines[currentLine].length();
                            }
                        }
                        break;
                    case 80: // 下箭头
                        if (currentLine < lines.size() - 1) {
                            currentLine++;
                            if (cursorPos > lines[currentLine].length()) {
                                cursorPos = lines[currentLine].length();
                            }
                        }
                        break;
                    case 75: // 左箭头
                        if (cursorPos > 0) {
                            cursorPos--;
                        } else if (currentLine > 0) {
                            currentLine--;
                            cursorPos = lines[currentLine].length();
                        }
                        break;
                    case 77: // 右箭头
                        if (cursorPos < lines[currentLine].length()) {
                            cursorPos++;
                        } else if (currentLine < lines.size() - 1) {
                            currentLine++;
                            cursorPos = 0;
                        }
                        break;
                }
            }
            // Ctrl+Enter 结束输入
            else if (ch == 10 && (GetKeyState(VK_CONTROL) & 0x8000)) {
                inputComplete = true;
                break;
            }
            else {
                switch (ch) {
                    case 8: // 退格键
                        if (cursorPos > 0) {
                            // 删除当前行的字符
                            lines[currentLine].erase(cursorPos - 1, 1);
                            cursorPos--;
                        } else if (currentLine > 0) {
                            // 合并到上一行
                            cursorPos = lines[currentLine - 1].length();
                            lines[currentLine - 1] += lines[currentLine];
                            lines.erase(lines.begin() + currentLine);
                            currentLine--;
                        }
                        break;
                        
                    case 13: // 回车键
                        if (cursorPos < lines[currentLine].length()) {
                            // 分割行
                            std::string newLine = lines[currentLine].substr(cursorPos);
                            lines[currentLine] = lines[currentLine].substr(0, cursorPos);
                            lines.insert(lines.begin() + currentLine + 1, newLine);
                        } else {
                            // 新空行
                            lines.insert(lines.begin() + currentLine + 1, "");
                        }
                        currentLine++;
                        cursorPos = 0;
                        break;
                        
                    default: // 普通字符
                        if (ch >= 32 && ch <= 126) { // 可打印字符
                            lines[currentLine].insert(cursorPos, 1, static_cast<char>(ch));
                            cursorPos++;
                        }
                        break;
                }
            }
            
            // 重新显示所有内容
            system("cls");
            printf("Brainfuck IDE(Press Ctrl + Enter to finish input)\n");
            printf("=====================================================================\n");
            
            for (int i = 0; i < lines.size(); i++) {
                printLineWithHighlight(lines[i], i + 1);
            }
            
            printf("=====================================================================\n");
            printf("Current row: %d, Position: %d", currentLine + 1, cursorPos + 1);
            
            // 重新定位光标
            moveCursor(4 + cursorPos, 2 + currentLine);
        }
        
        Sleep(10); // 小延迟减少CPU占用
    }
    
    // 恢复控制台模式
    SetConsoleMode(hStdin, originalMode);
    
    // 构建最终的输入字符串
    std::string input;
    for (const auto& line : lines) {
        input += line + "\n";
    }
    
    data.original = input;
    
    // 处理注释和过滤有效指令（保持原有逻辑）
    bool inMultiLineComment = false;
    bool inSingleLineComment = false;
    
    for (std::string::size_type i = 0; i < input.length(); i++) {
        char ch = input[i];
        char next_ch = (i + 1 < input.length()) ? input[i + 1] : '\0';
        
        // 处理多行注释
        if (!inSingleLineComment && ch == '/' && next_ch == '*') {
            inMultiLineComment = true;
            i++;
            continue;
        }
        if (inMultiLineComment && ch == '*' && next_ch == '/') {
            inMultiLineComment = false;
            i++;
            continue;
        }
        if (inMultiLineComment) {
            continue;
        }
        
        // 处理单行注释
        if (!inMultiLineComment && ch == '/' && next_ch == '/') {
            inSingleLineComment = true;
            i++;
            continue;
        }
        if (inSingleLineComment && ch == '\n') {
            inSingleLineComment = false;
            continue;
        }
        if (inSingleLineComment) {
            continue;
        }
        
        // 只保留有效的Brainfuck指令字符
        if (ch == '[' || ch == ']' || ch == '<' || ch == '>' || 
            ch == '.' || ch == ',' || ch == '+' || ch == '-') {
            data.filtered += ch;
        }
    }
    return data;
}

// 运行函数：执行Brainfuck程序
int run(std::string program) {
    BrainfuckCompiler bfc;
    bfc.loadCode(program);
    bfc.interpret();
    return 0;
}

void running(std::string pro){
    printf("\n%s\n", tr("run_results").c_str());
    int res=run(pro);
    if(res==0) printf("\n%s\n", tr("run_success").c_str());
    if(res==1) printf("\n%s\n", tr("pointer_error").c_str());
    if(res==2) printf("\n%s\n", tr("compile_error").c_str());
}

// 保存程序到文件（包含注释）
bool saveProgram(const std::string& filename, const ProgramData& programData) {
    std::string programDir = getExeDir();
#ifdef _WIN32
    programDir += "\\Program";
    // 创建Program目录（如果不存在）
    createDirectory(programDir);
    
    std::string fullPath = programDir + "\\" + filename + ".bf";
#else
    programDir += "/Program";
    // 创建Program目录（如果不存在）
    createDirectory(programDir);
    
    std::string fullPath = programDir + "/" + filename + ".bf";
#endif
    
    std::ofstream file(fullPath.c_str());
    if (file.is_open()) {
        // 保存原始程序（包含注释）
        file << "/* Brainfuck Program with Comments */\n";
        file << "/* Saved from Brainfuck IDE */\n\n";
        file << programData.original;
        file << "\n\n/* Filtered executable code: */\n";
        file << "/* " << programData.filtered << " */\n";
        
        file.close();
        printf("%s %s\n", tr("save_success").c_str(), fullPath.c_str());
        return true;
    } else {
        printf("%s\n", tr("save_failed").c_str());
        return false;
    }
}

// 从文件加载程序
ProgramData loadProgram(const std::string& filename) {
    ProgramData data;
    std::ifstream file(filename.c_str());
    if (file.is_open()) {
        std::string line;
        bool inCommentBlock = false;
        
        while (std::getline(file, line)) {
        	data.original += line + "\n";
            // 跳过文件头部的注释块
            if (line.find("/* Brainfuck Program with Comments */") != std::string::npos ||
                line.find("/* Saved from Brainfuck IDE */") != std::string::npos ||
                line.find("/* Filtered executable code: */") != std::string::npos) {
                continue;
            }
            
            // 如果是注释行，跳过
            if (line.length() >= 2 && line.substr(0, 2) == "/*") {
                inCommentBlock = true;
            }
            if (inCommentBlock && line.length() >= 2 && line.substr(line.length()-2) == "*/") {
                inCommentBlock = false;
                continue;
            }
            if (inCommentBlock) {
                continue;
            }
            
            // 同时过滤有效指令
            for (std::string::size_type i = 0; i < line.length(); i++) {
                char ch = line[i];
                if (ch == '[' || ch == ']' || ch == '<' || ch == '>' || 
                    ch == '.' || ch == ',' || ch == '+' || ch == '-') {
                    data.filtered += ch;
                }
            }
        }
        file.close();
    } else {
        printf("无法打开文件: %s\n", filename.c_str());
    }
    return data;
}

void createEditor(ProgramData programData = ProgramData()) {
    bool editing = true;
    
    while(editing) {
        clearScreen();
        /*
        printf("\n%s\n", tr("editor_commands").c_str());
        printf("%s\n", tr("edit_program").c_str());
        printf("%s\n", tr("run_program").c_str());
        printf("%s\n", tr("save_program").c_str());
        printf("%s\n", tr("clear_program").c_str());
        printf("%s\n", tr("show_filtered").c_str());
        printf("%s\n", tr("language_settings_editor").c_str());
        printf("%s\n", tr("back_menu").c_str());
        printf("%s", tr("select_option").c_str());
        */
        char choice;
        WinConsoleMenu editorMenu(tr("editor_commands"));
        editorMenu.addOption(tr("edit_program"), [&choice]() {choice='1';});
        editorMenu.addOption(tr("run_program"), [&choice]() {choice='2';});
        editorMenu.addOption(tr("debug_program"), [&choice]() {choice='8';});
        editorMenu.addOption(tr("save_program"), [&choice]() {choice='3';});
        editorMenu.addOption(tr("clear_program"), [&choice]() {choice='4';});
        editorMenu.addOption(tr("show_filtered"), [&choice]() {choice='5';});
        editorMenu.addOption(tr("language_settings_editor"), [&choice]() {choice='6';});
        editorMenu.addOption(tr("back_menu"), [&choice]() {choice='7';});
        editorMenu.show();
        editorMenu.executeSelected();
        switch(choice) {
            case '1': {
            	clearScreen();
            	std::string sts = programData.original;
                programData = input_Bf(sts);
                break;
            }
            case '2':
                if (!programData.filtered.empty()) {
                	clearScreen();
                    running(programData.filtered);
                } else {
                    printf("%s\n", tr("program_empty").c_str());
                }
                pauseScreen();
                break;
            case '3':
                if (!programData.original.empty()) {
                	clearScreen();
                    printf("%s", tr("enter_filename").c_str());
                    std::string filename;
                    std::getline(std::cin,filename);
                    saveProgram(filename, programData);
                } else {
                    printf("%s\n", tr("program_empty").c_str());
                }
                pauseScreen();
                break;
            case '4':
            	clearScreen();
                programData.original.clear();
                programData.filtered.clear();
                printf("%s\n", tr("program_cleared").c_str());
                pauseScreen();
                break;
            case '5':
            	clearScreen();
                printf("%s %s\n", tr("filtered_code").c_str(), programData.filtered.c_str());
                pauseScreen();
                break;
            case '6':
                languageSettings();
                break;
            case '7':
            	clearScreen();
                editing = false;
                break;
            case '8': 
    	 	    if (!programData.filtered.empty()) {
                	clearScreen();
                    BrainfuckCompiler bfc;
				    bfc.loadCode(programData.filtered);
				    std::cout << "Debug mode. Press Enter to step through execution.\n";
	                while (bfc.step()) {
	                    bfc.printCurrentState();
	                    bfc.printMemoryState();
	                    std::cout << "Press Enter to continue...";
	                    std::cin.ignore();
	                }
                } else {
                    printf("%s\n", tr("program_empty").c_str());
                }
                pauseScreen();
                break;
            default:
            	clearScreen();
                printf("%s\n", tr("invalid_choice").c_str());
                pauseScreen();
                break;
        }
    }
}

void create(){
    createEditor();
}

void open(){
    std::string programDir = getExeDir();
#ifdef _WIN32
    programDir += "\\Program";
#else
    programDir += "/Program";
#endif
    
    // 使用递归搜索查找所有子目录中的 .bf 文件
    std::vector<std::string> bfFiles = DirectoryReader::getBFFilesRecursive(programDir);
    int fileCount = bfFiles.size();
    
    if (fileCount == 0) {
        printf("%s\n", tr("no_bf_files").c_str());
        pauseScreen();
        return;
    }
    
    // 使用WinConsoleMenu显示文件选择
    WinConsoleMenu fileMenu(trf("found_files", fileCount));
    
    for (std::vector<std::string>::size_type i = 0; i < bfFiles.size(); i++) {
        // 显示相对路径，去掉程序目录前缀
        std::string relativePath = bfFiles[i].substr(programDir.length() + 1);
        fileMenu.addOption(relativePath, [bfFiles, i, programDir]() {
            std::string selectedFile = bfFiles[i];
            printf("%s\n", trf("opening_file", selectedFile.c_str()).c_str());
            
            // 加载程序
            ProgramData programData = loadProgram(selectedFile);
            if (!programData.original.empty()) {
                createEditor(programData);
            } else {
                printf("%s\n", tr("file_empty").c_str());
                pauseScreen();
            }
        });
    }
    
    fileMenu.addOption(tr("back_to_main"), []() {});
    
    while (true) {
        int choice = fileMenu.show();
        if (choice == -1 || choice == static_cast<int>(bfFiles.size())) {
            break; // ESC或选择返回
        }
        fileMenu.executeSelected();
        break; // 执行后退出文件选择菜单
    }
}

int main(){
    // 初始化所有映射
    initializeTranslations();
    initializeLanguageNames();
    initializeColorNames();
    initializeColorMaps();
    
    // 创建Program目录
    std::string programDir = getExeDir();
#ifdef _WIN32
    programDir += "\\Program";
#else
    programDir += "/Program";
#endif
    createDirectory(programDir);
    
    // 加载保存的设置
    loadSettings();
    
    clearScreen();
    SetConsoleTitleA(tr("main_menu").c_str());
    WinConsoleMenu mainMenu(tr("main_menu"));
    mainMenu.addOption(tr("main_menu1"), create);
    mainMenu.addOption(tr("main_menu2"), open);
    mainMenu.addOption(tr("main_menu3"), editorSettings);
    mainMenu.addOption(tr("main_menu4"), abort);
    while (true) {
        int choice = mainMenu.show();
        if (choice == -1) {
            break; // ESC退出
        }
        mainMenu.executeSelected();
    }
    return 0;
}
