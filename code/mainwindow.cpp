#include <QString>
#include <QTextEdit>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QCheckBox>
#include <QListWidget>
#include <QObject>
#include <QTextStream>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <map>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <iomanip>

#include <QApplication>
#include <QFile>

#include "dialog.h"
#include "base.txt"

using std::string;
using std::cin;
using std::cout;
using std::vector;
using std::pair;
using std::map;
using std::to_string;

//std::ifstream fin;
//std::ofstream fout;

bool have_derivative = 0, have_make_polynom = 0, have_value_in_a_point = 0;
List_Of_Polynomials* polynoms_head = NULL, *cur_polynom = NULL;
List_Of_Polynomials* active_polynom_1 = NULL, *active_polynom_2 = NULL;
int active_polynom_num_1 = -1, active_polynom_num_2 = -1;
QListWidgetItem* polynom_1_item = NULL, *polynom_2_item = NULL;

void Del_Spaces(QString &ss) {
    QString tmp;
    for (int ii = 0; ii < ss.size(); ++ii) {
        if (ss[ii] != ' ') {
            tmp.push_back(ss[ii]);
        }
    }
    ss = tmp;
    return;
}

bool Right_Polynomial(QString s) {
    int n = s.size();
    QString error;
    if (n == 0) {
        error = "empty string";
        QMessageBox::warning(nullptr, "error", error);
        return 0;
    }
    bool start = 1, was_x = 0, was_stepen = 0, was_cf = 0;
    int i = 0;
    if (s[0] == '-') {
        ++i;
    }
    for (; i < n; ++i) {
        if (start) {
            if (s[i] >= 48 && s[i] <= 57) {
                while (i < n) {
                    if (s[i] >= 48 && s[i] <= 57) {
                        ++i;
                    } else {
                        break;
                    }
                }
                if (i == n) {
                    return 1;
                }
                --i;
                was_cf = 1;
            } else if (s[i] >= 97 && s[i] <= 122) {
                was_x = 1;
            } else {
                error = "error: expexted coefficient or variable lowercase letter, but found: \"";
                error.push_back(s[i]);
                error.push_back("\" (symbol number: " + QString::number(i + 1) + ')');
                QMessageBox::warning(nullptr, "error", error);
                return 0;
            }
            start = 0;
        } else if (was_cf) {
            if (s[i] == '+' || s[i] == '-') {
                start = 1;
            } else if (s[i] >= 97 && s[i] <= 122) {
                was_x = 1;
            } else if (s[i] == '*') {
                start = 1;
            } else {
                error = "error: expected variable lowercase letter, or \"*\", or\"+\", or\"-\", but found: \"";
                error.push_back(s[i]);
                error.push_back("\" (symbol number: " + QString::number(i + 1) + ')');
                QMessageBox::warning(nullptr, "error", error);
                return 0;
            }
            was_cf = 0;
        } else if (was_x) {
            if (s[i] == '^') {
                ++i;
                if (i == n) {
                    error = "expected degree, but end of polynomial found(end of string)";
                    QMessageBox::warning(nullptr, "error", error);
                    return 0;
                }
                if (s[i] >= 48 && s[i] <= 57) {
                    while (i < n) {
                        if (s[i] <= 57 && s[i] >= 48) {
                            ++i;
                        } else {
                            break;
                        }
                    }
                    if (i == n) {
                        return 1;
                    }
                    --i;
                    was_stepen = 1;
                } else {
                    error = "expected degree, but found: \"";
                    error.push_back(s[i]);
                    error.push_back("\" (symbol number: " + QString::number(i + 1) + ')');
                    QMessageBox::warning(nullptr, "error", error);
                    return 0;
                }
            } else if (s[i] == '*') {
                start = 1;
            } else if (s[i] == '+' || s[i] == '-') {
                start = 1;
            } else if (s[i] >= 97 && s[i] <= 122) {
                continue;
            } else {
                error = R"(expected "+", or "-", or "*", or "^", or variable lowercase letter, but found: ")";
                error.push_back(s[i]);
                error.push_back("\" (symbol number: " + QString::number(i + 1) + ')');
                QMessageBox::warning(nullptr, "error", error);
                return 0;
            }
            was_x = 0;
        } else if (was_stepen) {
            if (s[i] == '-' || s[i] == '+' || s[i] == '*') {
                start = 1;
            } else if (s[i] >= 97 || s[i] <= 122) {
                was_x = 1;
            } else {
                error = R"(expected "+", or "-", or "*", or variable lowercase letter, but found: ")";
                error.push_back(s[i]);
                error.push_back("\" (symbol number: " + QString::number(i + 1) + ')');
                QMessageBox::warning(nullptr, "error", error);
                return 0;
            }
            was_stepen = 0;
        }
    }
    if (start) {
        error = "expected variable or coefficient, but end of string found";
        QMessageBox::critical(nullptr, "error", error);
        return 0;
    }
    return 1;
}

void Push_Back_Letter_List(char cc, Letter_List* &l) {
    Letter_List* p = new Letter_List;
    p->letter = cc;
    if (l == NULL) {
        p->next = NULL;
    } else {
        p->next = l->next;
        l->next = p;
    }
    l = p;
    return;
}

void Push_Back_Terms_List(vector<int> degrees_mass, Terms_List* &l) {
    Terms_List* p = new Terms_List;
    p->degrees.resize(27);
    for (int i = 0; i < 27; ++i) {
        p->degrees[i] = degrees_mass[i];
    }
    if (l == NULL) {
        p->next = NULL;
    } else {
        p->next = l->next;
        l->next = p;
    }
    l = p;
    return;
}

void Delete_Polynomial_After_This(List_Of_Polynomials* &l, bool delete_this) {
    List_Of_Polynomials* q = l;
    if (l == NULL) {
        return;
    }

    if (!delete_this) {
        l = l->next;
        q->next = l->next;
    } else {
        q = l->next;
    }
    Letter_List* q2_letters_tmp, *q1_letters_tmp = l->letter_list;
    while (q1_letters_tmp != NULL) {
        q2_letters_tmp = q1_letters_tmp->next;
        delete q1_letters_tmp;
        q1_letters_tmp = q2_letters_tmp;
    }

    Terms_List *q1_terms_tmp = l->terms_list, *q2_terms_tmp;
    while (q1_terms_tmp != NULL) {
        q2_terms_tmp = q1_terms_tmp->next;
        delete q1_terms_tmp;
        q1_terms_tmp = q2_terms_tmp;
    }
    delete l;
    l = q;
    return;
}

vector<vector<int>> Make_Mass_Out_Of_Polynomial(List_Of_Polynomials* l) {
    vector<vector<int>> terms;
    Terms_List* q_tmp_terms = l->terms_list;
    for (int i = 0; l->terms_list != NULL; ++i) {
        terms.push_back(l->terms_list->degrees);
        l->terms_list = l->terms_list->next;
    }
    l->terms_list = q_tmp_terms;
    return terms;
}

List_Of_Polynomials* Make_Polynomial_Out_Of_Mass(vector<vector<int>> terms) {
    List_Of_Polynomials* p = new List_Of_Polynomials;
    if (terms.size() == 0) {
        p->terms_list = NULL;
        p->letter_list = NULL;
        p->letter_number = 0;
        p->max_degree_sum = 0;
        return p;
    }

    int max_degree_sum = 0;
    for (int i = 0; i < 26; ++i) {
        max_degree_sum += terms[0][i];
    }
    p->max_degree_sum = max_degree_sum;

    p->terms_list = NULL;
    Terms_List* q_tmp_terms = NULL;
    for (int i = 0; i < terms.size(); ++i) {
        Push_Back_Terms_List(terms[i], p->terms_list);
        if (!i) {
            q_tmp_terms = p->terms_list;
        }
    }
    p->terms_list = q_tmp_terms;

    bool letters[26];
    for (int i = 0; i < 26; ++i) {
        letters[i] = 0;
    }
    for (int i = 0; i < terms.size(); ++i) {
        for (int ii = 0; ii < 26; ++ii) {
            if (terms[i][ii]) {
                letters[ii] = 1;
            }
        }
    }

    p->letter_list = NULL;
    p->letter_number = 0;
    Letter_List* q_tmp_letter = NULL;
    for (int i = 0; i < 26; ++i) {
        if (letters[i]) {
            Push_Back_Letter_List(i + 'a', p->letter_list);
            if (!p->letter_number) {
                q_tmp_letter = p->letter_list;
            }
            ++p->letter_number;
        }
    }
    p->letter_list = q_tmp_letter;
    return p;
}

List_Of_Polynomials* Make_Polynomial_Out_Of_String(QString s) {
    List_Of_Polynomials* p;

    int terms_cnt = 1;
    for (int i = 1; i < s.size(); ++i) {
        if (s[i] == '-' || s[i] == '+') {
            ++terms_cnt;
        }
    }
    vector<vector<int>> aa(terms_cnt);
    for (int i = 0; i < terms_cnt; ++i) {
        aa[i].resize(27, 0);
        aa[i][26] = 1;
    }
    int index_left = 0, index_right = 1;
    for (int i = 0; i < terms_cnt; ++i) {
        for (index_right = index_left + 1; index_right < s.size(); ++index_right) {
            if (s[index_right] == '+' || s[index_right] == '-') {
                break;
            }
        }
        if (s[index_left] == '-') {
            aa[i][26] = -1;
            ++index_left;
        } else if (s[index_left] == '+') {
            ++index_left;
        }
        bool start = 1, was_x = 0;
        char prev_x;
        for (int ii = index_left; ii < index_right;) {
            if (start) {
                if (s[ii] >= 48 && s[ii] <= 57) {
                    int cur_cf = 0;
                    do {
                        if (!(s[ii] >= 48 && s[ii] <= 57)) {
                            break;
                        }
                        cur_cf *= 10;
                        cur_cf += s[ii].unicode() - 48;
                        ++ii;
                    } while (ii < s.size());
                    if (ii == s.size()) {
                        aa[i][26] *= cur_cf;
                        continue;
                    }
                    if (s[ii] == '*') {
                        ++ii;
                    }
                    aa[i][26] *= cur_cf;
                    continue;
                } else {
                    prev_x = s[ii].unicode();
                    was_x = 1;
                    ++ii;
                }
                start = 0;
            } else if (was_x) {
                if (s[ii] == '^') {
                    ++ii;
                    int cur_stepen = 0;
                    do {
                        if (!(s[ii] >= 48 && s[ii] <= 57)) {
                            break;
                        }
                        cur_stepen *= 10;
                        cur_stepen += s[ii].unicode() - 48;
                        ++ii;
                    } while (ii < s.size());
                    aa[i][prev_x - 97] += cur_stepen;
                    if (ii == s.size()) {
                        was_x = 0;
                        continue;
                    }
                    if (s[ii] == '*') {
                        ++ii;
                    }
                } else {
                    ++aa[i][prev_x - 97];
                    if (s[ii] == '*') {
                        ++ii;
                    }
                }
                start = 1;
                was_x = 0;
            }
        }
        if (was_x) {
            ++aa[i][prev_x - 97];
        }
        index_left = index_right;
    }
    for (int i = 0; i < aa.size(); ++i) {
        for (int ii = i + 1; ii < aa.size(); ++ii) {
            bool equal = 1;
            for (int iii = 0; iii < 26; ++iii) {
                if (aa[i][iii] != aa[ii][iii]) {
                    equal = 0;
                    break;
                }
            }
            if (equal) {
                aa[i][26] += aa[ii][26];
                aa.erase(aa.begin() + ii);
            }
        }
    }
    vector<vector<int>> copy(aa.size());
    for (int i = 0; i < aa.size(); ++i) {
        copy[i].resize(27);
    }
    vector<pair<int, int>> tmp_to_sort(aa.size());
    for (int i = 0; i < aa.size(); ++i) {
        int sum_degree = 0;
        for (int ii = 0; ii < 26; ++ii) {
            sum_degree += aa[i][ii];
        }
        tmp_to_sort[i] = std::make_pair(sum_degree, i);
    }
    std::sort(tmp_to_sort.begin(), tmp_to_sort.end());
    reverse(tmp_to_sort.begin(), tmp_to_sort.end());
    for (int i = 0; i < aa.size(); ++i) {
        for (int ii = 0; ii < 27; ++ii) {
            copy[i][ii] = aa[tmp_to_sort[i].second][ii];
        }
    }
    for (int i = 0; i < copy.size(); ++i) {
        if (copy[i][26] == 0) {
            copy.erase(copy.begin() + i);
            --i;
        }
    }

    p = Make_Polynomial_Out_Of_Mass(copy);
    return p;
}

void Push_BacK_Polynomial(List_Of_Polynomials* p, List_Of_Polynomials* &l, bool push_front) {
    if (!push_front) {
        if (l == NULL) {
            p->next = NULL;
            p->serial_number = 0;
        } else {
            p->next = l->next;
            l->next = p;
            p->serial_number = l->serial_number + 1;
        }
    } else {
        p->next = l;
        p->serial_number = 0;
    }
    l = p;
    List_Of_Polynomials* tmp = l->next;
    while (tmp != NULL) {
        ++tmp->serial_number;
        tmp = tmp->next;
    }
    return;
}

List_Of_Polynomials* Plus_Polynomials(List_Of_Polynomials* l1, List_Of_Polynomials* l2, bool minus) {
    vector<vector<int>> l1_terms, l2_terms;
    l1_terms = Make_Mass_Out_Of_Polynomial(l1);
    l2_terms = Make_Mass_Out_Of_Polynomial(l2);

    if (minus) {
        for (int i = 0; i < l2_terms.size(); ++i) {
            l2_terms[i][26] *= -1;
        }
    }

    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = l1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    q_tmp_letter = l2->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    int cnt_used_letters = 0;
    int our_index;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            our_index = i;
            ++cnt_used_letters;
        }
    }

    vector<vector<int>> res;
    if (cnt_used_letters == 1) {
        vector<int> monom(27, 0);
        for (int i = 0, j = 0; i + j < l1_terms.size() + l2_terms.size();) {
            if (i == l1_terms.size()) {
                res.push_back(l2_terms[j]);
                ++j;
            } else if (j == l2_terms.size()) {
                res.push_back(l1_terms[i]);
                ++i;
            } else if (l1_terms[i][our_index] > l2_terms[j][our_index]) {
                monom[our_index] = l1_terms[i][our_index];
                monom[26] = l1_terms[i][26];
                res.push_back(monom);
                ++i;
            } else if (l1_terms[i][our_index] < l2_terms[j][our_index]) {
                monom[our_index] = l2_terms[j][our_index];
                monom[26] = l2_terms[j][26];
                res.push_back(monom);
                ++j;
            } else {
                monom[our_index] = l2_terms[j][our_index];
                monom[26] = l2_terms[j][26] + l1_terms[i][26];
                if (monom[26] != 0) {
                    res.push_back(monom);
                }
                ++j;
                ++i;
            }
        }
    } else {
        map<vector<int>, int> p_terms;
        vector<int> monom;
        int coef;
        for (int i = 0; i < l1_terms.size(); ++i) {
            monom = l1_terms[i];
            monom.pop_back();
            coef = l1_terms[i][26];
            if (p_terms.find(monom) == p_terms.end()) {
                p_terms.insert({monom, coef});
            } else {
                p_terms.find(monom)->second += coef;
            }
        }
        for (int i = 0; i < l2_terms.size(); ++i) {
            monom = l2_terms[i];
            monom.pop_back();
            coef = l2_terms[i][26];
            if (p_terms.find(monom) == p_terms.end()) {
                p_terms.insert({monom, coef});
            } else {
                p_terms.find(monom)->second += coef;
            }
        }
        l1_terms.clear();
        map<vector<int>, int>::iterator q_map = p_terms.begin();
        while (q_map != p_terms.end()) {
            if (q_map->second == 0) {
                ++q_map;
                continue;
            }
            l1_terms.push_back(q_map->first);
            l1_terms.back().push_back(q_map->second);
            ++q_map;
        }
        vector<pair<int, int>> sorted(p_terms.size());
        for (int i = 0; i < l1_terms.size(); ++i) {
            int sum_degree = 0;
            for (int ii = 0; ii < 26; ++ii) {
                sum_degree += l1_terms[i][ii];
            }
            sorted[i].first = sum_degree;
            sorted[i].second = i;
        }
        std::sort(sorted.begin(), sorted.end());
        reverse(sorted.begin(), sorted.end());
        vector<vector<int>> res;
        for (int i = 0; i < p_terms.size(); ++i) {
            res.push_back(l1_terms[sorted[i].second]);
        }
    }

    List_Of_Polynomials* p = Make_Polynomial_Out_Of_Mass(res);
    return p;
}

List_Of_Polynomials* Multiply_Polynomials(List_Of_Polynomials* l1, List_Of_Polynomials* l2) {
    vector<vector<int>> l1_terms, l2_terms;
    l1_terms = Make_Mass_Out_Of_Polynomial(l1);
    l2_terms = Make_Mass_Out_Of_Polynomial(l2);

    map<vector<int>, int> p_map_terms;
    for (int i = 0; i < l1_terms.size(); ++i) {
        for (int j = 0; j < l2_terms.size(); ++j) {
            vector<int> tmp(26);
            for (int ii = 0; ii < 26; ++ii) {
                tmp[ii] = l1_terms[i][ii] + l2_terms[j][ii];
            }
            int coef = l1_terms[i][26] * l2_terms[j][26];
            if (p_map_terms.find(tmp) == p_map_terms.end()) {
                p_map_terms.insert(std::make_pair(tmp, coef));
            } else {
                p_map_terms.find(tmp)->second += coef;
            }
        }
    }
    vector<vector<int>> p_terms;
    map<vector<int>, int>::iterator q_tmp_map = p_map_terms.begin();
    while (q_tmp_map != p_map_terms.end()) {
        if (q_tmp_map->second != 0) {
            p_terms.push_back(q_tmp_map->first);
            p_terms[p_terms.size() - 1].push_back(q_tmp_map->second);
        }
        ++q_tmp_map;
    }

    vector<pair<int, int>> sorted(p_terms.size());
    for (int i = 0; i < p_terms.size(); ++i) {
        int sum_degree = 0;
        for (int ii = 0; ii < 26; ++ii) {
            sum_degree += p_terms[i][ii];
        }
        sorted[i].first = sum_degree;
        sorted[i].second = i;
    }
    std::sort(sorted.begin(), sorted.end());
    reverse(sorted.begin(), sorted.end());
    vector<vector<int>> res;
    for (int i = 0; i < p_terms.size(); ++i) {
        res.push_back(p_terms[sorted[i].second]);
    }

    List_Of_Polynomials* p = Make_Polynomial_Out_Of_Mass(res);
    return p;
}

pair<List_Of_Polynomials*, List_Of_Polynomials*> Divide_Polynomials(List_Of_Polynomials* l1, List_Of_Polynomials* l2) {
    int our_index;
    if (l1->letter_list != NULL) {
        our_index = l1->letter_list->letter - 97;
    }
    if (l2->letter_list != NULL) {
        our_index = l2->letter_list->letter - 97;
    }

    vector<vector<int>> l1_terms, l2_terms;
    l1_terms = Make_Mass_Out_Of_Polynomial(l1);
    l2_terms = Make_Mass_Out_Of_Polynomial(l2);
    vector<vector<int>> quotient, remainder;
    vector<int> multiplier(27, 0);
    while (l1_terms.size() > 0) {
        if(l1_terms[0][26] % l2_terms[0][26] || l1_terms[0][our_index] < l2_terms[0][our_index]) {
            remainder = l1_terms;
            break;
        }
        multiplier[26] = l1_terms[0][26] / l2_terms[0][26];
        multiplier[our_index] = l1_terms[0][our_index] - l2_terms[0][our_index];
        quotient.push_back(multiplier);
        vector<vector<int>> res;
        for (int i = 0, j = 0; i < l1_terms.size() || j < l2_terms.size();) {
            if (i == l1_terms.size()) {
                res.push_back(l2_terms[j]);
                res[res.size() - 1][26] *= multiplier[26] * (-1);
                ++j;
            } else if (j == l2_terms.size()) {
                res.push_back(l1_terms[i]);
                ++i;
            } else if (l1_terms[i][our_index] == l2_terms[j][our_index] + multiplier[our_index]) {
                l1_terms[i][26] -= l2_terms[j][26] * multiplier[26];
                if (l1_terms[i][26] == 0) {
                    ++i;
                }
                ++j;
            } else {
                if (l1_terms[i][our_index] > l2_terms[j][our_index] + multiplier[our_index]) {
                    res.push_back(l1_terms[i]);
                    ++i;
                } else {
                    res.push_back(l2_terms[j]);
                    res[res.size() - 1][26] *= -1 * multiplier[26];
                    res[res.size() - 1][our_index] += multiplier[our_index];
                    ++j;
                }
            }
        }
        l1_terms = res;
    }

    List_Of_Polynomials* p1, *p2;
    p1 = Make_Polynomial_Out_Of_Mass(quotient);
    p2 = Make_Polynomial_Out_Of_Mass(remainder);
    return std::make_pair(p1, p2);
}

int Value_In_Point(vector<vector<int>> terms, vector<int> variables_value) {
    int ans = 0;
    for (int i = 0; i < terms.size(); ++i) {
        int cur_monom = 1;
        for (int ii = 0; ii < 26; ++ii) {
            cur_monom *= (int)std::pow(variables_value[ii], terms[i][ii]);
        }
        cur_monom *= terms[i][26];
        ans += cur_monom;
    }
    return ans;
}

vector<int> Roots(List_Of_Polynomials* l) {
    vector<vector<int>> l_terms;
    l_terms = Make_Mass_Out_Of_Polynomial(l);
    vector<int> roots;
    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = l->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    int cnt_used_letters = 0;
    int our_index;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            our_index = i;
            ++cnt_used_letters;
        }
    }
    if (cnt_used_letters > 1) {
        return roots;                          // надо переместить в доп. функцию
    }

    if (l_terms.size() == NULL) {
        return roots;
    }
    vector<int> our_variables_value(26, 0);
    int min_degree = l_terms.back()[our_index];
    if (min_degree > 0) {
        roots.push_back(0);
    }
    for (int ii = 0; ii < l_terms.size(); ++ii) {
        l_terms[ii][our_index] -= min_degree;
    }
    for (int i = 1; i <= abs(l_terms.back()[26]); ++i) {
        if (l_terms.back()[26] % i == 0) {
            our_variables_value[our_index] = i;
            if (Value_In_Point(l_terms, our_variables_value) == 0) {
                roots.push_back(i);
            }
            our_variables_value[our_index] = -i;
            if (Value_In_Point(l_terms, our_variables_value) == 0) {
                roots.push_back(-i);
            }
        }
    }
    return roots;
}

List_Of_Polynomials* Derivative(vector<vector<int>> terms, int our_index) {
    for (int i = 0; i < terms.size(); ++i) {
        terms[i][26] *= terms[i][our_index];
        --terms[i][our_index];
    }
    for (int i = 0; i < terms.size(); ++i) {
        for (int ii = i + 1; ii < terms.size(); ++ii) {
            if (terms[i][our_index] == terms[ii][our_index]) {
                terms[i][26] += terms[ii][26];
                terms.erase(terms.begin() + ii);
                --ii;
            }
        }
    }
    for (int i = 0; i < terms.size(); ++i) {
        if (terms[i][26] == 0) {
            terms.erase(terms.begin() + i);
            --i;
        }
    }
    List_Of_Polynomials* p = Make_Polynomial_Out_Of_Mass(terms);
    return p;
}

List_Of_Polynomials* Get_polynomial(int num_in_row) {
    if (num_in_row < 0) {
        return NULL;
    }
    List_Of_Polynomials* p = polynoms_head;
    for (int i = 0; i < num_in_row; ++i) {
        p = p->next;
    }
    return p;
}

/*void Show_Polynomial(List_Of_Polynomials* l) {
    if (l == NULL) {
        return;
    }
    Terms_List* p = l->terms_list;
    if (p == NULL) {
        cout << 0 << "\n";
        return;
    }
    if (p->degrees[26] < 0) {
        cout << "-";
    }
    if (abs(p->degrees[26]) != 1) {
        cout << abs(p->degrees[26]);
    } else {
        int tmp = 0;
        for (int i = 0; i < 26; ++i) {
            tmp += p->degrees[i];
        }
        if (tmp == 0) {
            cout << 1;
        }
    }
    for (int i = 0; i < 26; ++i) {
        if (p->degrees[i] > 0) {
            if (p->degrees[i] == 1) {
                cout << (char)(i + 97);
            } else {
                cout << (char)(i + 97) << '^' << p->degrees[i];
            }
        }
    }
    p = p->next;
    while (p != NULL) {
        if (p->degrees[26] > 0) {
            cout << "+";
        } else {
            cout << "-";
        }
        if (abs(p->degrees[26]) != 1) {
            cout << abs(p->degrees[26]);
        } else {
            int tmp = 0;
            for (int i = 0; i < 26; ++i) {
                tmp += p->degrees[i];
            }
            if (tmp == 0) {
                cout << 1;
            }
        }
        for (int i = 0; i < 26; ++i) {
            if (p->degrees[i] == 1) {
                cout << (char)(i + 97);
            } else if (p->degrees[i] > 1) {
                cout << (char)(i + 97) << '^' << p->degrees[i];
            }
        }
        p = p->next;
    }
    cout << "\n";
    return;
}*/

QString Make_String_Out_Of_Polynom(List_Of_Polynomials* l) {
    QString ss;
    if (l == NULL) {
        return ss;
    }
    Terms_List* p = l->terms_list;
    if (p == NULL) {
        ss.push_back('0');
        return ss;
    }
    if (p->degrees[26] < 0) {
        ss.push_back('-');
    }
    if (abs(p->degrees[26]) != 1) {
        ss.push_back(QString::number(abs(p->degrees[26])));
    } else {
        int tmp = 0;
        for (int i = 0; i < 26; ++i) {
            tmp += p->degrees[i];
        }
        if (tmp == 0) {
            ss.push_back('1');
        }
    }
    for (int i = 0; i < 26; ++i) {
        if (p->degrees[i] > 0) {
            if (p->degrees[i] == 1) {
                ss.push_back((char)(i + 97));
            } else {
                ss.push_back((char)(i + 97));
                ss.push_back('^');
                ss.push_back(QString::number(p->degrees[i]));
            }
        }
    }
    p = p->next;
    while (p != NULL) {
        if (p->degrees[26] > 0) {
            ss.push_back(" + ");
        } else {
            ss.push_back(" - ");
        }
        if (abs(p->degrees[26]) != 1) {
            ss.push_back(QString::number(abs(p->degrees[26])));
        } else {
            int tmp = 0;
            for (int i = 0; i < 26; ++i) {
                tmp += p->degrees[i];
            }
            if (tmp == 0) {
                ss.push_back('1');
            }
        }
        for (int i = 0; i < 26; ++i) {
            if (p->degrees[i] == 1) {
                ss.push_back((char)(i + 97));
            } else if (p->degrees[i] > 1) {
                ss.push_back((char)(i + 97));
                ss.push_back('^');
                ss.push_back(QString::number(p->degrees[i]));
            }
        }
        p = p->next;
    }
    return ss;
}

void Zeroize_Items() {
    if (polynom_2_item != NULL) {
        polynom_2_item->setBackgroundColor(Qt::transparent);
        polynom_2_item = NULL;
    }
    if (polynom_1_item != NULL) {
        polynom_1_item->setBackgroundColor(Qt::transparent);
        polynom_1_item = NULL;
    }
    active_polynom_num_1 = active_polynom_num_2 = -1;
    active_polynom_1 = active_polynom_2 = NULL;
}

bool Is_Right_Values(QString ss) {
    QString error;
    if (ss == "") {
        error = "empty string";
        QMessageBox::critical(nullptr, "error", error);
        return 0;
    }
    if (ss[0] >= 48 && ss[0] <= 57) {
        error = "expected variable, but number found: \"" + ss[0] + "\", symbol nuber: 1";
        QMessageBox::critical(nullptr, "error", error);
        return 0;
    }
    vector<bool> used_letters(26, 0);
    bool was_variable = 0;
    for (int i = 0; i < ss.size(); ++i) {
        if (ss[i] >= 97 && ss[i] <= 122) {
            if (was_variable) {
                error = "expexted value, but variable found: \"" + ss[i] + "\", symbol number: " + QString::number(i + 1);
                QMessageBox::critical(nullptr, "error", error);
                return 0;
            }
            if (used_letters[ss[i].unicode() - 97]) {
                error = "double meet of variable \"" + ss[i] + "\", symbol number: " + QString::number(i + 1);
                QMessageBox::critical(nullptr, "error", error);
                return 0;
            }
            used_letters[ss[i].unicode() - 97] = 1;
            was_variable = 1;
        } else if (ss[i] < 48 || ss[i] > 57) {
            error = "expected number or variable, but \"" + ss[i] + "\" found, symbol number: " + QString::number(i + 1);
            QMessageBox::critical(nullptr, "error", error);
            return 0;
        } else {
            was_variable = 0;
        }
    }
    if (was_variable) {
        error = "expexted value, but end of string found";
        QMessageBox::critical(nullptr, "error", error);
        return 0;
    }
    return 1;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    //Dialog kek;
    //kek.setModal(true);
    //kek.exec();
    delete ui;
}



void MainWindow::on_pushButton_save_res_1_clicked() {
    QString ss;
    ss = ui->lineEdit->text();
    Del_Spaces(ss);
    if (have_derivative) {
        vector<bool> used_letters(26, 0);
        Letter_List* q_tmp_letter;
        q_tmp_letter = active_polynom_1->letter_list;
        while(q_tmp_letter != NULL) {
            used_letters[q_tmp_letter->letter - 'a'] = 1;
            q_tmp_letter = q_tmp_letter->next;
        }

        if (ss[0].unicode() - 'a' < 0 || ss[0].unicode() - 'a' > 25) {
            QMessageBox::critical(nullptr, "", "you need to enter variable");
            return;
        }
        int our_index = ss[0].unicode() - 'a';
        int step = 0;
        for (int i = 1; i < ss.size(); ++i) {
            if (ss[i].unicode() - '0' < 0 || ss[i].unicode() - '0' > 9) {
                QMessageBox::critical(nullptr, "", "you need to enter degree of derivative after variable");
                return;
            }
            step *= 10;
            step += ss[i].unicode() - '0';
        }

        vector<vector<int>> terms;
        terms = Make_Mass_Out_Of_Polynomial(active_polynom_1);
        for (int i = 0; i < step; ++i) {
            for (int ii = 0; ii < terms.size(); ++ii) {
                terms[ii][26] *= terms[ii][our_index];
                --terms[ii][our_index];
            }
        }
        for (int i = 0; i < terms.size(); ++i) {
            if (terms[i][26] == 0) {
                terms.erase(terms.begin() + i);
                --i;
            }
        }
        List_Of_Polynomials* p = Make_Polynomial_Out_Of_Mass(terms);
        QString ans = Make_String_Out_Of_Polynom(p);
        Delete_Polynomial_After_This(p, 1);
        ui->lineEdit->setText(ans);
        have_derivative = 0;
    } else if (have_make_polynom) {
        if (!Is_Right_Values(ss)) {
            return;
        }
        vector<int> letters_value(26, 0);
        vector<bool> used_letters(26, 0);
        Letter_List* q_tmp_letter;
        q_tmp_letter = active_polynom_1->letter_list;
        while(q_tmp_letter != NULL) {
            used_letters[q_tmp_letter->letter - 'a'] = 1;
            q_tmp_letter = q_tmp_letter->next;
        }
        int n = ss.size();
        for (int i = 0; i < n;) {
            int cur_index = ss[i].unicode() - 97;
            ++i;
            int cur_num = 0;
            for (; i < n; ++i) {
                if (ss[i] < 48 || ss[i] > 57) {
                    break;
                }
                cur_num *= 10;
                cur_num += ss[i].unicode() - 48;
            }
            letters_value[cur_index] = cur_num;
        }
        vector<vector<int>> terms;
        terms = Make_Mass_Out_Of_Polynomial(active_polynom_1);
        for (int i = 0; i < terms.size(); ++i) {
            for (int ii = 0; ii < 26; ++ii) {
                if (terms[i][ii] != 0 && letters_value[ii] != 0) {
                    terms[i][26] *= (int)pow(letters_value[ii], terms[i][ii]);
                    terms[i][ii] = 0;
                }
            }
        }
        for (int i = 0; i < terms.size(); ++i) {
            if (terms[i][26] == 0) {
                terms.erase(terms.begin() + i);
                --i;
            }
        }
        List_Of_Polynomials* p = Make_Polynomial_Out_Of_Mass(terms);
        QString ans = Make_String_Out_Of_Polynom(p);
        Delete_Polynomial_After_This(p, 1);
        ui->lineEdit->setText(ans);
        have_make_polynom = 0;
    } else if (have_value_in_a_point) {
        if (!Is_Right_Values(ss)) {
            return;
        }
        vector<int> letters_value(26, 0);
        vector<bool> used_letters(26, 0);
        Letter_List* q_tmp_letter;
        q_tmp_letter = active_polynom_1->letter_list;
        while(q_tmp_letter != NULL) {
            used_letters[q_tmp_letter->letter - 'a'] = 1;
            q_tmp_letter = q_tmp_letter->next;
        }
        int n = ss.size();
        for (int i = 0; i < n;) {
            int cur_index = ss[i].unicode() - 97;
            ++i;
            int cur_num = 0;
            for (; i < n; ++i) {
                if (ss[i] < 48 || ss[i] > 57) {
                    break;
                }
                cur_num *= 10;
                cur_num += ss[i].unicode() - 48;
            }
            letters_value[cur_index] = cur_num;
        }
        QString variables;
        for (int i = 0; i < 26; ++i) {
            if (used_letters[i] == 1 && letters_value[i] == 0) {
                variables.push_back(QChar(i + 97));
                variables.push_back(", ");
            }
        }
        variables.resize(variables.size() - 2);
        if (variables.size() > 0) {
            QString error;
            error = "you haven`t entered all variable values, you haven`t entered values of: " + variables;
            QMessageBox::critical(nullptr, "error", error);
            return;
        }
        vector<vector<int>> terms;
        int ans = 0;
        terms = Make_Mass_Out_Of_Polynomial(active_polynom_1);
        for (int i = 0; i < terms.size(); ++i) {
            for (int ii = 0; ii < 26; ++ii) {
                if (terms[i][ii] != 0) {
                    terms[i][26] *= (int)pow(letters_value[ii], terms[i][ii]);
                    terms[i][ii] = 0;
                }
            }
            ans += terms[i][26];
        }
        ui->lineEdit->setText(QString::number(ans));
        have_value_in_a_point = 0;
    } else {
        if (ss == "") {
            QMessageBox::critical(nullptr, "error", "empty string");
            return;
        }
        if (Right_Polynomial(ss)) {
            List_Of_Polynomials* p = Make_Polynomial_Out_Of_String(ss);
            cur_polynom = polynoms_head;
            if (polynoms_head == NULL) {
                Push_BacK_Polynomial(p, polynoms_head, 0);
                cur_polynom = polynoms_head;
            } else if (p->max_degree_sum > polynoms_head->max_degree_sum) {
                Push_BacK_Polynomial(p, polynoms_head, 1);
            } else {
                while (cur_polynom->next != NULL) {
                    if (cur_polynom->next->max_degree_sum > p->max_degree_sum) {
                        cur_polynom = cur_polynom->next;
                    } else {
                        break;
                    }
                }
                Push_BacK_Polynomial(p, cur_polynom, 0);
            }
            ss.clear();
            ss = (Make_String_Out_Of_Polynom(p));
            ss = QString::number(p->serial_number + 1) + ") " + ss;
            ui->listWidget->insertItem(p->serial_number, ss);
            for (int i = p->serial_number + 1; i < ui->listWidget->count(); ++i) {
                ss = ui->listWidget->item(i)->text();
                int num = 0;
                QString kek;
                int ii = 0;
                for (; ii < ss.size(); ++ii) {
                    if (ss[ii] == ')') {
                        break;
                    }
                    num *= 10;
                    num += ss[ii].unicode() - '0';
                }
                ++num;
                ii += 2;
                for (; ii < ss.size(); ++ii) {
                    kek += ss[ii];
                }
                ui->listWidget->item(i)->setText(QString::number(num) + ") " + kek);
            }
            ui->lineEdit->clear();
            /*if (p->serial_number >= active_polynom_num_1) {
                ++active_polynom_num_1;
            }
            if (p->serial_number >= active_polynom_num_2) {
                ++active_polynom_num_2;
            }*/
            Zeroize_Items();
            //ui->listWidget->currentItem()->setSelected(0);
        }
    }
}

void MainWindow::on_pushButton_plus_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    active_polynom_2 = Get_polynomial(active_polynom_num_2);
    if (active_polynom_2 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t two decided polynoms");
        return;
    }
    List_Of_Polynomials* p = Plus_Polynomials(active_polynom_1, active_polynom_2, 0);
    QString ss;
    ss = Make_String_Out_Of_Polynom(p);
    ui->lineEdit->setText(ss);
    Delete_Polynomial_After_This(p, 1);
}

void MainWindow::on_pushButton_minus_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    active_polynom_2 = Get_polynomial(active_polynom_num_2);
    if (active_polynom_2 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t two decided polynoms");
        return;
    }
    List_Of_Polynomials* p = Plus_Polynomials(active_polynom_1, active_polynom_2, 1);
    QString ss;
    ss = Make_String_Out_Of_Polynom(p);
    ui->lineEdit->setText(ss);
    Delete_Polynomial_After_This(p, 1);
}

void MainWindow::on_pushButton_multiply_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    active_polynom_2 = Get_polynomial(active_polynom_num_2);
    if (active_polynom_2 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided polynoms");
        return;
    }
    List_Of_Polynomials* p = Multiply_Polynomials(active_polynom_1, active_polynom_2);
    QString ss;
    ss = Make_String_Out_Of_Polynom(p);
    ui->lineEdit->setText(ss);
    Delete_Polynomial_After_This(p, 1);
}

void MainWindow::on_pushButton_divide_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    active_polynom_2 = Get_polynomial(active_polynom_num_2);
    if (active_polynom_2 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided two polynom");
        return;
    }
    if (active_polynom_2->terms_list == NULL) {
        QMessageBox::critical(nullptr, "error", "you tried to divide by zero");
        return;
    }
    if (active_polynom_1->terms_list == NULL) {
        ui->lineEdit->setText("0");
        ui->lineEdit_2->setText("0");
        return;
    }

    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = active_polynom_1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    q_tmp_letter = active_polynom_2->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    int cnt_used_letters = 0;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            ++cnt_used_letters;
        }
    }
    if (cnt_used_letters > 1) {
        QMessageBox::information(nullptr, "sorry", "you have chosen two polynoms with more than one variable, or with different variables, and our program can`t divide these polynoms");
        return;
    }

    pair<List_Of_Polynomials*, List_Of_Polynomials*> p = Divide_Polynomials(active_polynom_1, active_polynom_2);
    QString ss;
    ss = Make_String_Out_Of_Polynom(p.first);
    ui->lineEdit->setText(ss);
    ss = Make_String_Out_Of_Polynom(p.second);
    ui->lineEdit_2->setText(ss);
}

void MainWindow::on_pushButton_value_in_point_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    if (active_polynom_1 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided polynom");
        return;
    }
    if (active_polynom_1->terms_list == NULL) {
        ui->lineEdit->setText("0");
        return;
    }
    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = active_polynom_1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    QString ss = "";
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            ss.push_back(" ");
            ss.push_back((char)(i + 97));
            ss.push_back(",");
        }
    }
    if (ss.size() == 0) {
        ui->lineEdit->setText(QString::number(active_polynom_1->terms_list->degrees[26]));
        return;
    }
    ss.resize(ss.size() - 1);
    ss.push_back(".");
    QMessageBox::information(nullptr, "", "enter the variable values in the format: variable, value, variable, value... , for example: \"a 2 x 5 k 4\"; you should enter values of:" + ss);
    have_value_in_a_point = 1;
    ui->lineEdit->setText("");
}

void MainWindow::on_pushButton_roots_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    if (active_polynom_1 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided polynom");
        return;
    }
    if (active_polynom_1->letter_number == 0) {
        if (active_polynom_1->terms_list == NULL) {
            QMessageBox::critical(nullptr, "error", "you have decided 0");
            return;
        } else {
            QMessageBox::critical(nullptr, "error", "no roots");
            return;
        }
    }

    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = active_polynom_1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    int cnt_used_letters = 0;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            ++cnt_used_letters;
        }
    }
    if (cnt_used_letters > 1) {
        QMessageBox::information(nullptr, "sorry", "you have chosen polynom with more than one variable, and my program can`t find roots of this polynom");
        return;
    }

    vector<int> roots = Roots(active_polynom_1);
    QString ss = "";
    for (int i = 0; i < roots.size(); ++i) {
        ss.push_back(QString::number(roots[i]) + " ");
    }
    ui->lineEdit->setText(ss);
    if (ss.size() == 0) {
        QMessageBox::information(nullptr, "", "polynom have no integer roots");
    }
}

void MainWindow::on_pushButton_derivative_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    if (active_polynom_1 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided polynom");
        return;
    }
    if (active_polynom_1->terms_list == NULL) {
        ui->lineEdit->setText("0");
        return;
    }
    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = active_polynom_1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    QString ss;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            ss.push_back(" ");
            ss.push_back((char)(i + 97));
            ss.push_back(",");
        }
    }
    if (ss.size() == 0) {
        ui->lineEdit->setText("0");
        return;
    }
    have_derivative = 1;
    ui->lineEdit->clear();
}

void MainWindow::on_pushButton_save_res_2_clicked() {
    QString ss;
    ss = ui->lineEdit_2->text();
    Del_Spaces(ss);
    if (ss == "") {
        QMessageBox::critical(nullptr, "error", "empty string");
        return;
    }
    if (Right_Polynomial(ss)) {
        List_Of_Polynomials* p = Make_Polynomial_Out_Of_String(ss);
        cur_polynom = polynoms_head;
        if (polynoms_head == NULL) {
            Push_BacK_Polynomial(p, polynoms_head, 0);
            cur_polynom = polynoms_head;
        } else if (p->max_degree_sum > polynoms_head->max_degree_sum) {
            Push_BacK_Polynomial(p, polynoms_head, 1);
        } else {
            while (cur_polynom->next != NULL) {
                if (cur_polynom->next->max_degree_sum > p->max_degree_sum) {
                    cur_polynom = cur_polynom->next;
                } else {
                    break;
                }
            }
            Push_BacK_Polynomial(p, cur_polynom, 0);
        }
        ss.clear();
        ss = (Make_String_Out_Of_Polynom(p));
        ui->lineEdit_2->clear();
        ui->listWidget->insertItem(p->serial_number, ss);
    }
}

void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    if (polynom_2_item != NULL) {
        polynom_2_item->setBackgroundColor(Qt::transparent);
    }
    polynom_2_item = polynom_1_item;
    polynom_1_item = current;
    if (polynom_2_item != NULL) {
        polynom_2_item->setBackgroundColor(Qt::yellow);
    }
    if (polynom_1_item != NULL) {
        polynom_1_item->setBackgroundColor(Qt::green);
    }
    active_polynom_num_2 = active_polynom_num_1;
    active_polynom_num_1 = ui->listWidget->currentRow();
    if (active_polynom_num_1 == active_polynom_num_2) {
        active_polynom_num_2 = -1;
    }
}

void MainWindow::on_pushButton_make_polynom_out_of_existing_clicked() {
    active_polynom_1 = Get_polynomial(active_polynom_num_1);
    if (active_polynom_1 == NULL) {
        QMessageBox::critical(nullptr, "error", "you haven`t decided two polynoms");
        return;
    }
    if (active_polynom_1->terms_list == NULL) {
        ui->lineEdit->setText("0");
        return;
    }
    vector<bool> used_letters(26, 0);
    Letter_List* q_tmp_letter;
    q_tmp_letter = active_polynom_1->letter_list;
    while(q_tmp_letter != NULL) {
        used_letters[q_tmp_letter->letter - 'a'] = 1;
        q_tmp_letter = q_tmp_letter->next;
    }
    QString ss;
    for (int i = 0; i < 26; ++i) {
        if (used_letters[i]) {
            ss.push_back(" ");
            ss.push_back((char)(i + 97));
            ss.push_back(",");
        }
    }
    if (ss.size() == 0) {
        ui->lineEdit->setText(QString::number(active_polynom_1->terms_list->degrees[26]));
        return;
    }
    have_make_polynom = 1;
    ui->lineEdit->setText("");
    QMessageBox::information(nullptr, "", "enter the variable values in the format: variable, value, variable, value... , for example: \"a 2 x 5 k 4\", you can enter values of:" + ss);
}

void MainWindow::on_pushButton_clear_clicked() {
    ui->lineEdit->clear();
    //Zeroize_Items();
}

void MainWindow::on_pushButton_delete_clicked() {
    if (active_polynom_num_1 == 0) {
        Delete_Polynomial_After_This(polynoms_head, 1);
    } else {
        active_polynom_1 = Get_polynomial(active_polynom_num_1 - 1);
        Delete_Polynomial_After_This(active_polynom_1, 0);
    }
    if (active_polynom_num_2 > active_polynom_num_1) {
        --active_polynom_num_2;
    }
    --active_polynom_num_1;
    if (active_polynom_num_1 == -1) {
        ++active_polynom_num_1;
    }
    //Zeroize_Items();
    ui->listWidget->takeItem(ui->listWidget->currentRow());
    if (ui->listWidget->currentRow() < 0) {
        return;
    }
    QString ss;
    for (int i = ui->listWidget->currentRow(); i < ui->listWidget->count(); ++i) {
        ss = ui->listWidget->item(i)->text();
        int num = 0;
        QString kek;
        int ii = 0;
        for (; ii < ss.size(); ++ii) {
            if (ss[ii] == ')') {
                break;
            }
            num *= 10;
            num += ss[ii].unicode() - '0';
        }
        --num;
        ii += 2;
        for (; ii < ss.size(); ++ii) {
            kek += ss[ii];
        }
        ui->listWidget->item(i)->setText(QString::number(num) + ") " + kek);
    }
    //ui->listWidget->currentItem()->setSelected(0);
}

void MainWindow::on_pushButton_delete_all_clicked() {
    while (polynoms_head != NULL) {
        Delete_Polynomial_After_This(polynoms_head, 1);
    }
    ui->listWidget->clear();
}

void MainWindow::on_pushButton_import_clicked() {
    QFile file("C:\\prak_polynomials\base.txt");
    if (!file.exists()) {
        //cout << "fuck u";
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //QTextStream stream(&file);
        QString ss;
        if (file.atEnd()) {
            //cout << "what?";
        }
        while(!file.atEnd()) {
            //cout << "lol\n";
            ss = file.readLine();
            List_Of_Polynomials* p = Make_Polynomial_Out_Of_String(ss);
            cur_polynom = polynoms_head;
            if (polynoms_head == NULL) {
                Push_BacK_Polynomial(p, polynoms_head, 0);
                cur_polynom = polynoms_head;
            } else if (p->max_degree_sum > polynoms_head->max_degree_sum) {
                Push_BacK_Polynomial(p, polynoms_head, 1);
            } else {
                while (cur_polynom->next != NULL) {
                    if (cur_polynom->next->max_degree_sum > p->max_degree_sum) {
                        cur_polynom = cur_polynom->next;
                    } else {
                        break;
                    }
                }
                Push_BacK_Polynomial(p, cur_polynom, 0);
            }
            ui->listWidget->insertItem(p->serial_number, ss);
        }
    } else {
        //cout << "loshara\n";
    }
}
