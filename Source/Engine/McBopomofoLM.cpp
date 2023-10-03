// Copyright (c) 2022 and onwards The McBopomofo Authors.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include "McBopomofoLM.h"
#include <algorithm>
#include <iterator>
#include <float.h>

namespace McBopomofo {

McBopomofoLM::McBopomofoLM()
{
}

McBopomofoLM::~McBopomofoLM()
{
    m_languageModel.close();
    m_userPhrases.close();
    m_excludedPhrases.close();
    m_phraseReplacement.close();
    m_associatedPhrases.close();
}

void McBopomofoLM::loadLanguageModel(const char* languageModelDataPath)
{
    if (languageModelDataPath) {
        m_languageModel.close();
        m_languageModel.open(languageModelDataPath);
    }
}

bool McBopomofoLM::isDataModelLoaded()
{
    return m_languageModel.isLoaded();
}

void McBopomofoLM::loadAssociatedPhrases(const char* associatedPhrasesPath)
{
    if (associatedPhrasesPath) {
        m_associatedPhrases.close();
        m_associatedPhrases.open(associatedPhrasesPath);
    }
}

bool McBopomofoLM::isAssociatedPhrasesLoaded()
{
    return m_associatedPhrases.isLoaded();
}

void McBopomofoLM::loadUserPhrases(const char* userPhrasesDataPath,
    const char* excludedPhrasesDataPath)
{
    if (userPhrasesDataPath) {
        m_userPhrases.close();
        m_userPhrases.open(userPhrasesDataPath);
    }
    if (excludedPhrasesDataPath) {
        m_excludedPhrases.close();
        m_excludedPhrases.open(excludedPhrasesDataPath);
    }
}

void McBopomofoLM::loadPhraseReplacementMap(const char* phraseReplacementPath)
{
    if (phraseReplacementPath) {
        m_phraseReplacement.close();
        m_phraseReplacement.open(phraseReplacementPath);
    }
}

std::vector<Formosa::Gramambular2::LanguageModel::Unigram> McBopomofoLM::getUnigrams(const std::string& key)
{
    if (key == " ") {
        std::vector<Formosa::Gramambular2::LanguageModel::Unigram> spaceUnigrams;
        spaceUnigrams.emplace_back(" ", 0);
        return spaceUnigrams;
    }

    std::vector<Formosa::Gramambular2::LanguageModel::Unigram> allUnigrams;
    std::vector<Formosa::Gramambular2::LanguageModel::Unigram> userUnigrams;

    std::unordered_set<std::string> excludedValues;
    std::unordered_set<std::string> insertedValues;

    if (m_excludedPhrases.hasUnigrams(key)) {
        std::vector<Formosa::Gramambular2::LanguageModel::Unigram> excludedUnigrams = m_excludedPhrases.getUnigrams(key);
        transform(excludedUnigrams.begin(), excludedUnigrams.end(),
            inserter(excludedValues, excludedValues.end()),
            [](const Formosa::Gramambular2::LanguageModel::Unigram& u) { return u.value(); });
    }

    if (m_userPhrases.hasUnigrams(key)) {
        std::vector<Formosa::Gramambular2::LanguageModel::Unigram> rawUserUnigrams = m_userPhrases.getUnigrams(key);
        userUnigrams = filterAndTransformUnigrams(rawUserUnigrams, excludedValues, insertedValues);
    }

    if (m_languageModel.hasUnigrams(key)) {
        std::vector<Formosa::Gramambular2::LanguageModel::Unigram> rawGlobalUnigrams = m_languageModel.getUnigrams(key);
        allUnigrams = filterAndTransformUnigrams(rawGlobalUnigrams, excludedValues, insertedValues);
    }

    allUnigrams.insert(allUnigrams.begin(), userUnigrams.begin(), userUnigrams.end());
    return allUnigrams;
}

bool McBopomofoLM::hasUnigrams(const std::string& key)
{
    if (key == " ") {
        return true;
    }

    if (!m_excludedPhrases.hasUnigrams(key)) {
        return m_userPhrases.hasUnigrams(key) || m_languageModel.hasUnigrams(key);
    }

    return getUnigrams(key).size() > 0;
}

std::string McBopomofoLM::getReading(const std::string_view& value)
{
    std::vector<std::string> records = m_languageModel.getReadings(value);
    
    double highScore = -DBL_MAX;
    std::string highScoringValue;
    for (std::string record : records) {
        std::vector<std::string_view> parts = split(record, ' ');
        if (parts.size() == 3) {
            double score = std::stod(std::string(parts[2]));
            if (score > highScore) {
                highScoringValue = std::string(parts[0]);
                highScore = score;
            }
        }
    }
    
    return highScoringValue;
}

void McBopomofoLM::setPhraseReplacementEnabled(bool enabled)
{
    m_phraseReplacementEnabled = enabled;
}

bool McBopomofoLM::phraseReplacementEnabled()
{
    return m_phraseReplacementEnabled;
}

void McBopomofoLM::setExternalConverterEnabled(bool enabled)
{
    m_externalConverterEnabled = enabled;
}

bool McBopomofoLM::externalConverterEnabled()
{
    return m_externalConverterEnabled;
}

void McBopomofoLM::setExternalConverter(std::function<std::string(std::string)> externalConverter)
{
    m_externalConverter = externalConverter;
}

std::vector<Formosa::Gramambular2::LanguageModel::Unigram> McBopomofoLM::filterAndTransformUnigrams(const std::vector<Formosa::Gramambular2::LanguageModel::Unigram> unigrams, const std::unordered_set<std::string>& excludedValues, std::unordered_set<std::string>& insertedValues)
{
    std::vector<Formosa::Gramambular2::LanguageModel::Unigram> results;

    for (auto&& unigram : unigrams) {
        // excludedValues filters out the unigrams with the original value.
        // insertedValues filters out the ones with the converted value
        std::string originalValue = unigram.value();
        if (excludedValues.find(originalValue) != excludedValues.end()) {
            continue;
        }

        std::string value = originalValue;
        if (m_phraseReplacementEnabled) {
            std::string replacement = m_phraseReplacement.valueForKey(value);
            if (replacement != "") {
                value = replacement;
            }
        }
        if (m_externalConverterEnabled && m_externalConverter) {
            std::string replacement = m_externalConverter(value);
            value = replacement;
        }
        if (insertedValues.find(value) == insertedValues.end()) {
            results.emplace_back(value, unigram.score());
            insertedValues.insert(value);
        }
    }
    return results;
}

const std::vector<std::string> McBopomofoLM::associatedPhrasesForKey(const std::string& key)
{
    return m_associatedPhrases.valuesForKey(key);
}

bool McBopomofoLM::hasAssociatedPhrasesForKey(const std::string& key)
{
    return m_associatedPhrases.hasValuesForKey(key);
}

std::vector<std::string_view> McBopomofoLM::split(const std::string_view& str, char delim) {
    std::vector<std::string_view> strings;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string_view::npos) {
        end = str.find(delim, start);
        strings.push_back(std::string_view(str.substr(start, end - start)));
    }
    return strings;
}

} // namespace McBopomofo
