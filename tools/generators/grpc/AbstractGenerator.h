/**
 * @copyright Copyright (c) 2020 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Loïc Touraine
 *
 * @file
 * @brief description of file
 * @date 2020-09-23
 */

#ifndef ABSTRACTGENERATOR_H
#define ABSTRACTGENERATOR_H
#include <xpcf/component/ComponentBase.h>

#include "interfaces/IRPCGenerator.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

namespace fs = boost::filesystem;

class AbstractGenerator : public org::bcom::xpcf::ComponentBase, virtual public IRPCGenerator
{
public:
    AbstractGenerator(std::map<std::string,std::string> componentInfosMap);
    ~AbstractGenerator() override;
    void unloadComponent () override final;
    void setDestinationFolder(const std::string & folder) override;
    void setGenerateMode(const GenerateMode & mode = GenerateMode::STD_COUT) override;

protected:
    GenerateMode m_mode;
    fs::path m_folder;

};

#endif // ABSTRACTGENERATOR_H
