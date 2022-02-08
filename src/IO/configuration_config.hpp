/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2021 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/
#ifndef __BITPIT_CONFIGURATION_CONFIG_HPP__
#define __BITPIT_CONFIGURATION_CONFIG_HPP__

#include <memory>
#include <map>
#include <string>
#include <vector>

namespace bitpit {

// Configuration storage
class Config
{

public:
    typedef std::map<std::string, std::string> Attributes;

    struct Option {
        std::string value;
        Attributes attributes;
    };

    typedef std::map<std::string, Option> Options;

    typedef Config Section;
    typedef std::vector<Section *> MultiSection;
    typedef std::vector<const Section *> ConstMultiSection;
    typedef std::multimap<std::string, std::unique_ptr<Config>> Sections;

    Config(bool multiSections = false);
    Config(const Config &other);
    Config(Config &&other) = default;

    Config & operator=(Config other);
    Config & operator=(Config &&other) = default;

    virtual ~Config() = default;

    void swap(Config &other);

    bool isMultiSectionsEnabled() const;

    const std::string & get(const std::string &key) const;
    std::string get(const std::string &key, const std::string &fallback) const;
    void set(const std::string &key, const std::string &value);

    std::string getAttribute(const std::string &key, const std::string &name) const;
    std::string getAttribute(const std::string &key, const std::string &name, const std::string &fallback) const;
    void setAttribute(const std::string &key, const std::string &name, const std::string &value);

    int getOptionCount() const;
    Options & getOptions();
    const Options & getOptions() const;
    Option & getOption(const std::string &key);
    const Option & getOption(const std::string &key) const;
    bool hasOption(const std::string &key) const;
    void addOption(const std::string &key, const Option &option);
    void addOption(const std::string &key, Option &&option);
    void addOption(const std::string &key, const std::string &value);
    void addOption(const std::string &key, std::string &&value);
    void addOption(const std::string &key, const std::string &value, const Attributes &attributes);
    void addOption(const std::string &key, std::string &&value, Attributes &&attributes);
    bool removeOption(const std::string &key);

    int getSectionCount() const;
    int getSectionCount(const std::string &key) const;
    Sections & getSections();
    const Sections & getSections() const;
    MultiSection getSections(const std::string &key);
    ConstMultiSection getSections(const std::string &key) const;
    bool hasSection(const std::string &key) const;
    Section & getSection(const std::string &key);
    const Section & getSection(const std::string &key) const;
    Section & addSection(const std::string &key);
    bool removeSection(const std::string &key);

    void clear();

    void dump(std::ostream &out, int indentLevel = 0) const;

    Section & operator[](const std::string &key);
    const Section & operator[](const std::string &key) const;

    template<typename T>
    T get(const std::string &key) const;

    template<typename T>
    T get(const std::string &key, const T &fallback) const;

    template<typename T>
    void set(const std::string &key, const T &value);

    template<typename T>
    T getAttribute(const std::string &key, const std::string &name) const;

    template<typename T>
    T getAttribute(const std::string &key, const std::string &name, const T &fallback) const;

    template<typename T>
    void setAttribute(const std::string &key, const std::string &name, const T &value);

protected:
    bool m_multiSections;

private:
    std::unique_ptr<Options> m_options;
    std::unique_ptr<Sections> m_sections;

};

}

#include "configuration_config.tpp"

#endif
