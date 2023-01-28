template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}


template <class Function>
void RunTestImpl(Function func, const string& func_name) {
    func();
    cerr << func_name << " OK"s << endl;
}


// -------- Начало модульных тестов поисковой системы ----------

//-Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestWordAddandFindWordinDoc() {
    const int doc_id1 = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content1 = "big wild cat walk in the city"s;
    const string content2 = "not all heroes wear capes in the city"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings1 = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 11, 4 };

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::REMOVED, ratings3);
        auto found_docs = server.FindTopDocuments("city"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, doc_id1);
        ASSERT_EQUAL(doc1.id, doc_id2);
        found_docs = server.FindTopDocuments("bell"s);
        ASSERT_EQUAL(found_docs.size(), 0);
    }
}


//-Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords() {
    const int doc_id1 = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content1 = "wild cat big walk in the words"s;
    const string content2 = "not all heroes wear city capes"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings1 = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 11, 4 };
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        auto found_docs = server.FindTopDocuments("city words -big"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        for (auto doc : found_docs) {
            cerr << doc.id << " " << doc.relevance << " " << doc.rating << endl;
        }
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id2);
        found_docs = server.FindTopDocuments("-city -words -big"s);
        ASSERT_EQUAL(found_docs.size(), 0);
    }
}


//-Матчинг документов. При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть //соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestMatching() {
    const int doc_id = 1;
    const int doc_id2 = 2;
    const string content = "big wild cat walk in the city"s;
    const string content2 = "not all  wear capes heroes in the city"s;
    const vector<int> ratings = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        tuple<vector<string>, DocumentStatus> res = server.MatchDocument("heroes socks wear", 2);
        vector<string> v{ "heroes","wear" };
        ASSERT_EQUAL(v, get<0>(res));
    }
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        tuple<vector<string>, DocumentStatus> res = server.MatchDocument("-heroes socks wear", 2);
        ASSERT(get<0>(res).empty());
    }
}


//-Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortRelevance() {
    const int doc_id = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content = "big wild cat walk in the city"s;
    const string content2 = "not all heroes wear capes in the city"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 22, 4 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city cat words big wild"s);
        ASSERT(found_docs[0].relevance >= found_docs[1].relevance && found_docs[1].relevance >= found_docs[2].relevance);
    }
}


//-Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestRatingComputing() {
    const int doc_id = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content = "big wild cat walk in the city"s;
    const string content2 = "not all heroes wear capes in the city"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 22, 4 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city cat words big wild"s);
        ASSERT_EQUAL(found_docs.size(), 3);
        ASSERT_EQUAL(found_docs[1].rating, 11);
    }
}


//-Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicate() {
    const int doc_id1 = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content1 = "wild cat big walk in the words"s;
    const string content2 = "not all heroes wear city capes"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings1 = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 11, 4 };
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city words -big", [](int document_id, DocumentStatus status, int rating) {return document_id == 3 && status == DocumentStatus::ACTUAL && rating < 100; });
        ASSERT_EQUAL(found_docs.size(), 1);
        ASSERT_EQUAL(found_docs[0].id, doc_id3);
    }
}


//-Поиск документов, имеющих заданный статус.
void TestStatus() {
    const int doc_id1 = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const int doc_id4 = 4;
    const int doc_id5 = 5;
    const string content1 = "wild cat big walk in the words"s;
    const string content2 = "not all heroes wear city capes"s;
    const string content3 = "minus words not easy at big all"s;
    const string content4 = "good old days in da city"s;
    const string content5 = "asta la vista baby"s;
    const vector<int> ratings1 = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 11, 4 };
    const vector<int> ratings4 = { 1, 15, 2 };
    const vector<int> ratings5 = { 1, 1, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::IRRELEVANT, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::BANNED, ratings3);
        server.AddDocument(doc_id4, content4, DocumentStatus::ACTUAL, ratings4);
        server.AddDocument(doc_id5, content5, DocumentStatus::REMOVED, ratings5);
        auto found_docs = server.FindTopDocuments("city words -big", DocumentStatus::ACTUAL);
        ASSERT_EQUAL(found_docs.size(), 1);
        found_docs = server.FindTopDocuments("city words", DocumentStatus::BANNED);
        ASSERT_EQUAL(found_docs[0].id, doc_id3);
        ASSERT_EQUAL(found_docs.size(), 1);
        found_docs = server.FindTopDocuments("city heroes", DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL(found_docs[0].id, doc_id2);
        ASSERT_EQUAL(found_docs.size(), 1);
        found_docs = server.FindTopDocuments("baby", DocumentStatus::REMOVED);
        ASSERT_EQUAL(found_docs[0].id, doc_id5);
        ASSERT_EQUAL(found_docs.size(), 1);
    }

}


//-Корректное вычисление релевантности найденных документов.
void TestRelevance() {
    const int doc_id1 = 1;
    const int doc_id2 = 2;
    const int doc_id3 = 3;
    const string content1 = "wild cat big walk in the words"s;
    const string content2 = "not all heroes wear city capes"s;
    const string content3 = "minus words not easy at all"s;
    const vector<int> ratings1 = { 18, 3, 4 };
    const vector<int> ratings2 = { 7, 11, 4 };
    const vector<int> ratings3 = { 7, 11, 4 };
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city words -big"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        for (auto doc : found_docs) {
            cerr << doc.id << " " << doc.relevance << " " << doc.rating << endl;
        }
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc1.id, doc_id3);
        ASSERT(doc1.relevance < 0.068 && doc1.relevance>0.06);
    }
}


// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 1;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}



// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestWordAddandFindWordinDoc);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatching);
    RUN_TEST(TestSortRelevance);
    RUN_TEST(TestRatingComputing);
    RUN_TEST(TestRelevance);
    RUN_TEST(TestStatus);
    RUN_TEST(TestPredicate);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();

    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}