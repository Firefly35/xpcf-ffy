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
 * @date 2020-02-10
 */
#include <iostream>

#include <cxxopts.hpp>

#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace
#include <cppast/libclang_parser.hpp> // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>         // for visit()
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_variable.hpp>
#include <cppast/cpp_template.hpp>
#include <cppast/cpp_array_type.hpp>
#include "ClassDescriptor.h"
#include <xpcf/api/IComponentManager.h>
#include <xpcf/core/helpers.h>
#include "GRPCProtoGenerator.h"
#include "GRPCFlatBufferGenerator.h"
#include "ProxyGenerator.h"
#include "ServerGenerator.h"
#include <boost/log/core/core.hpp>
namespace xpcf = org::bcom::xpcf;


/// usage sample:
/// --database_file /path/to/database_dir/compile_commands.json --database_dir /path/to/database_dir/ --remove_comments_in_macro --std c++1z

// print help options
void print_help(const cxxopts::Options& options)
{
    std::cout << options.help({"", "parsing", "generation"}) << '\n';
}

// print error message
void print_error(const std::string& msg)
{
    std::cerr << msg << '\n';
}

// prints the AST entry of a cpp_entity (base class for all entities),
// will only print a single line
void parse_entity(const cppast::cpp_entity_index& idx, std::ostream& out, const cppast::cpp_entity& e)
{
    std::vector<ClassDescriptor> interfaces;
    std::map<std::string,ClassDescriptor> classes;
    // print name and the kind of the entity
    if (!e.name().empty())
        out << e.name();
    else
        out << "<anonymous>";
    out << " (" << cppast::to_string(e.kind()) << ")";

    // print whether or not it is a definition
    if (cppast::is_definition(e))
        out << " [definition]";

    // print number of attributes
    if (!e.attributes().empty())
        out << " [" << e.attributes().size() << " attribute(s)]";

    if (e.kind() == cppast::cpp_entity_kind::class_t) {
        out << " [" << e.name() << "] is class"<< '\n';
        ClassDescriptor c(e);
        c.parse(idx);
        if (c.isInterface()) {
            interfaces.push_back(c);
        }
        else {
            if (!xpcf::mapContains(classes,c.getName())) {
                classes.insert(std::make_pair(c.getName(),c));
                out << "Found user defined type = "<<c.getName()<<'\n';
            }
        }
    }

    if (e.kind() == cppast::cpp_entity_kind::language_linkage_t)
        // no need to print additional information for language linkages
        out << '\n';
    else if (e.kind() == cppast::cpp_entity_kind::namespace_t)
    {
        // cast to cpp_namespace
        auto& ns = static_cast<const cppast::cpp_namespace&>(e);
        // print whether or not it is inline
        if (ns.is_inline())
            out << " [inline]";
        out << '\n';
    }
    else
    {
        // print the declaration of the entity
        // it will only use a single line
        // derive from code_generator and implement various callbacks for printing
        // it will print into a std::string
        class code_generator : public cppast::code_generator
        {
            std::string str_;                 // the result
            bool        was_newline_ = false; // whether or not the last token was a newline
            // needed for lazily printing them

        public:
            code_generator(const cppast::cpp_entity& e)
            {
                // kickoff code generation here
                cppast::generate_code(*this, e);
            }

            // return the result
            const std::string& str() const noexcept
            {
                return str_;
            }

        private:
            // called to retrieve the generation options of an entity
            generation_options do_get_options(const cppast::cpp_entity&,
                                              cppast::cpp_access_specifier_kind) override
            {
                // generate declaration only
                return code_generator::declaration;
            }

            // no need to handle indentation, as only a single line is used
            void do_indent() override {}
            void do_unindent() override {}

            // called when a generic token sequence should be generated
            // there are specialized callbacks for various token kinds,
            // to e.g. implement syntax highlighting
            void do_write_token_seq(cppast::string_view tokens) override
            {
                if (was_newline_)
                {
                    // lazily append newline as space
                    str_ += ' ';
                    was_newline_ = false;
                }
                // append tokens
                str_ += tokens.c_str();
            }

            // called when a newline should be generated
            // we're lazy as it will always generate a trailing newline,
            // we don't want
            void do_write_newline() override
            {
                was_newline_ = true;
            }

        } generator(e);
        // print generated code
        out << ": `" << generator.str() << '`' << '\n';
    }
    for (auto & c : interfaces) {
        // check every typedescriptor in each method of the interface is known in classes map or is a builtin type??
        // Note : we must find a message/serialized buffer for each type in each interface
        std::map<IRPCGenerator::MetadataType,std::string> metadata;
        metadata [IRPCGenerator::MetadataType::INTERFACENAMESPACE] = "";
        metadata = xpcf::getComponentManagerInstance()->resolve<IRPCGenerator>()->generate(c, metadata);
        xpcf::getComponentManagerInstance()->resolve<IRPCGenerator>("proxy")->generate(c, metadata);
        xpcf::getComponentManagerInstance()->resolve<IRPCGenerator>("server")->generate(c, metadata);
    }
}

// parses the AST of a file
void parse_ast(const cppast::cpp_entity_index& idx, std::ostream& out, const cppast::cpp_file& file)
{
    // print file name
    out << "AST for '" << file.name() << "':\n";
    std::string prefix; // the current prefix string
    // recursively visit file and all children
    cppast::visit(file, [&](const cppast::cpp_entity& e, cppast::visitor_info info) {
        if (e.kind() == cppast::cpp_entity_kind::file_t || cppast::is_templated(e)
                || cppast::is_friended(e))
            // no need to do anything for a file,
            // templated and friended entities are just proxies, so skip those as well
            // return true to continue visit for children
            return true;
        else if (info.event == cppast::visitor_info::container_entity_exit)
        {
            // we have visited all children of a container,
            // remove prefix
            prefix.pop_back();
            prefix.pop_back();
        }
        else
        {
            out << prefix; // print prefix for previous entities
            // calculate next prefix
            if (info.last_child)
            {
                if (info.event == cppast::visitor_info::container_entity_enter)
                    prefix += "  ";
                out << "+-";
            }
            else
            {
                if (info.event == cppast::visitor_info::container_entity_enter)
                    prefix += "| ";
                out << "|-";
            }

            parse_entity(idx, out, e);
        }

        return true;
    });
}

// parse a file
std::unique_ptr<cppast::cpp_file> parse_file(const cppast::libclang_compile_config& config,
                                             const cppast::diagnostic_logger&       logger,
                                             const std::string& filename, bool fatal_error,
                                             cppast::cpp_entity_index & idx)

{
    // the parser is used to parse the entity
    // there can be multiple parser implementations
    cppast::libclang_parser parser(type_safe::ref(logger));
    // parse the file
    auto file = parser.parse(idx, filename, config);
    if (fatal_error && parser.error())
        return nullptr;
    return file;
}

int set_compile_options(const cxxopts::ParseResult & options, cppast::libclang_compile_config & config)
{
    if (options.count("verbose"))
        config.write_preprocessed(true);

    if (options.count("fast_preprocessing"))
        config.fast_preprocessing(true);

    if (options.count("remove_comments_in_macro"))
        config.remove_comments_in_macro(true);

    if (options.count("include_directory"))
        for (auto& include : options["include_directory"].as<std::vector<std::string>>())
            config.add_include_dir(include);
    if (options.count("macro_definition"))
        for (auto& macro : options["macro_definition"].as<std::vector<std::string>>())
        {
            auto equal = macro.find('=');
            auto name  = macro.substr(0, equal);
            if (equal == std::string::npos)
                config.define_macro(std::move(name), "");
            else
            {
                auto def = macro.substr(equal + 1u);
                config.define_macro(std::move(name), std::move(def));
            }
        }
    if (options.count("macro_undefinition"))
        for (auto& name : options["macro_undefinition"].as<std::vector<std::string>>())
            config.undefine_macro(name);
    if (options.count("feature"))
        for (auto& name : options["feature"].as<std::vector<std::string>>())
            config.enable_feature(name);

    // the compile_flags are generic flags
    cppast::compile_flags flags;
    if (options.count("gnu_extensions"))
        flags |= cppast::compile_flag::gnu_extensions;
    if (options.count("msvc_extensions"))
        flags |= cppast::compile_flag::ms_extensions;
    if (options.count("msvc_compatibility"))
        flags |= cppast::compile_flag::ms_compatibility;

    if (options["std"].as<std::string>() == "c++98")
        config.set_flags(cppast::cpp_standard::cpp_98, flags);
    else if (options["std"].as<std::string>() == "c++03")
        config.set_flags(cppast::cpp_standard::cpp_03, flags);
    else if (options["std"].as<std::string>() == "c++11")
        config.set_flags(cppast::cpp_standard::cpp_11, flags);
    else if (options["std"].as<std::string>() == "c++14")
        config.set_flags(cppast::cpp_standard::cpp_14, flags);
    else if (options["std"].as<std::string>() == "c++1z")
        config.set_flags(cppast::cpp_standard::cpp_1z, flags);
    else
    {
        print_error("invalid value '" + options["std"].as<std::string>() + "' for std flag");
        return 1;
    }
    return 0;
}

template <class FileParser>
void parse_database(FileParser& parser, const cppast::libclang_compilation_database& database, const cxxopts::ParseResult & options)
{
    static_assert(std::is_same<typename FileParser::parser, cppast::libclang_parser>::value,
            "must use the libclang parser");
    struct data_t
    {
        FileParser&                          parser;
        const cppast::libclang_compilation_database& database;
        const cxxopts::ParseResult options;
    } data{parser, database, options};

    cppast::detail::for_each_file(database, &data, [](void* ptr, std::string file) {
        auto& data = *static_cast<data_t*>(ptr);

        cppast::libclang_compile_config config(data.database, file);
        set_compile_options(data.options,config);
        data.parser.parse(std::move(file), std::move(config));
    });
}

template <typename Callback>
int parse_database(const std::string & databasePath, const cppast::cpp_entity_index& index, const cxxopts::ParseResult & options, Callback cb)
try
{

    cppast::libclang_compilation_database database(databasePath); // the compilation database

    // simple_file_parser allows parsing multiple files and stores the results for us
    cppast::simple_file_parser<cppast::libclang_parser> parser(type_safe::ref(index));
    try
    {
        parse_database(parser, database,options); // parse all files in the database
    }
    catch (cppast::libclang_error& ex)
    {
        std::cerr << "fatal libclang error: " << ex.what() << '\n';
        return 1;
    }

    if (parser.error())
        // a non-fatal parse error
        // error has been logged to stderr
        return 1;

    for (auto& file : parser.files())
        cb(index,std::cout,file);
    return 0;
}
catch (std::exception& ex)
{
    std::cerr << ex.what() << '\n';
    return 1;
}


int main(int argc, char* argv[])
try
{
    boost::log::core::get()->set_logging_enabled(false);
    SRef<xpcf::IComponentManager> cmpMgr = xpcf::getComponentManagerInstance();

    cxxopts::Options option_list("xpcf_grpc_gen",
                                 "xpcf_grpc_gen - The commandline interface to the grpc generators for xpcf.\n");
    // clang-format off
    option_list.add_options()
            ("h,help", "display this help and exit")
            ("version", "display version information and exit")
            ("v,verbose", "be verbose when parsing")
            ("g,generator", "the message format to use : either 'flatbuffers' or 'protobuf'",
             cxxopts::value<std::string>()->default_value("flatbuffers"))
            ("fatal_errors", "abort program when a parser error occurs, instead of doing error correction")
            ("file", "the file that is being parsed",
             cxxopts::value<std::string>());
    option_list.add_options("parsing")
            ("database_dir", "set the directory where a 'compile_commands.json' file is located containing build information",
             cxxopts::value<std::string>())
            ("database_file", "set the file name whose configuration will be used regardless of the current file name",
             cxxopts::value<std::string>())
            ("std", "set the C++ standard (c++98, c++03, c++11, c++14, c++1z (experimental))",
             cxxopts::value<std::string>()->default_value(cppast::to_string(cppast::cpp_standard::cpp_latest)))
            ("I,include_directory", "add directory to include search path",
             cxxopts::value<std::vector<std::string>>())
            ("D,macro_definition", "define a macro on the command line",
             cxxopts::value<std::vector<std::string>>())
            ("U,macro_undefinition", "undefine a macro on the command line",
             cxxopts::value<std::vector<std::string>>())
            ("f,feature", "enable a custom feature (-fXX flag)",
             cxxopts::value<std::vector<std::string>>())
            ("gnu_extensions", "enable GNU extensions (equivalent to -std=gnu++XX)")
            ("msvc_extensions", "enable MSVC extensions (equivalent to -fms-extensions)")
            ("msvc_compatibility", "enable MSVC compatibility (equivalent to -fms-compatibility)")
            ("fast_preprocessing", "enable fast preprocessing, be careful, this breaks if you e.g. redefine macros in the same file!")
            ("remove_comments_in_macro", "whether or not comments generated by macro are kept, enable if you run into errors");
    option_list.add_options("generation")
            ("o,output", "set the destination folder where the generated grpc files will be created. The folder is created if it doesn't already exists",
             cxxopts::value<std::string>());

    // clang-format on
    waitForUserInput();
    auto options = option_list.parse(argc, argv);
    if (options.count("help"))
        print_help(option_list);
    else if (options.count("version"))
    {
        std::cout << "cppast version MYVERSION \n";
        std::cout << "Copyright (C) Jonathan Müller 2017-2019 <jonathanmueller.dev@gmail.com>\n";
        std::cout << '\n';
        std::cout << "Using libclang version  \n";
    }
    else {
        cmpMgr->bindLocal<IRPCGenerator, GRPCFlatBufferGenerator, xpcf::IComponentManager::Singleton>();
        cmpMgr->bindLocal<IRPCGenerator, ProxyGenerator, xpcf::IComponentManager::Singleton>("proxy");
        cmpMgr->bindLocal<IRPCGenerator, ServerGenerator, xpcf::IComponentManager::Singleton>("server");
        if (options["generator"].as<std::string>() == "protobuf") {
            cmpMgr->bindLocal<IRPCGenerator, GRPCProtoGenerator, xpcf::IComponentManager::Singleton>();
        }
        else if (options["generator"].as<std::string>() != "flatbuffers") {
            print_error("invalid value " + options["generator"].as<std::string>() + " for generator option. See usage :");
            print_help(option_list);
            return 1;
        }
        if (options.count("output")) {
            auto rpcGenerator = cmpMgr->resolve<IRPCGenerator>();
            auto proxyGenerator = cmpMgr->resolve<IRPCGenerator>("proxy");
            auto serverGenerator = cmpMgr->resolve<IRPCGenerator>("server");
            rpcGenerator->setGenerateMode(IRPCGenerator::GenerateMode::FILE);
            proxyGenerator->setGenerateMode(IRPCGenerator::GenerateMode::FILE);
            serverGenerator->setGenerateMode(IRPCGenerator::GenerateMode::FILE);
            rpcGenerator->setDestinationFolder(options["output"].as<std::string>());
            proxyGenerator->setDestinationFolder(options["output"].as<std::string>());
            serverGenerator->setDestinationFolder(options["output"].as<std::string>());
        }

        if (!options.count("file") || options["file"].as<std::string>().empty()) {
            if (options.count("database_dir") && !options["database_dir"].as<std::string>().empty()) {
                std::cout<<"File argument is missing : parsing every file listed in database"<<std::endl;
                cppast::cpp_entity_index idx;
                parse_database(options["database_dir"].as<std::string>(),idx,options, &parse_ast);
            }
            else {
                print_error("missing one of file or database dir argument");
                return 1;
            }
        }
        else {
            // the compile config stores compilation flags
            cppast::libclang_compile_config config;
            if (options.count("database_dir"))
            {
                cppast::libclang_compilation_database database(
                            options["database_dir"].as<std::string>());
                if (options.count("database_file"))
                    config
                            = cppast::libclang_compile_config(database,
                                                              options["database_file"].as<std::string>());
                else
                    config
                            = cppast::libclang_compile_config(database, options["file"].as<std::string>());
            }

            int result = set_compile_options(options,config);
            if (result !=0) {
                return result;
            }

            // the logger is used to print diagnostics
            cppast::stderr_diagnostic_logger logger;
            if (options.count("verbose"))
                logger.set_verbose(true);
            cppast::cpp_entity_index idx;// the entity index is used to resolve cross references in the AST
            auto file = parse_file(config, logger, options["file"].as<std::string>(),
                    options.count("fatal_errors") == 1, idx);
            if (!file)
                return 2;
            parse_ast(idx, std::cout, *file);
        }
    }
}
catch (const cppast::libclang_error& ex)
{
    print_error(std::string("[fatal parsing error] ") + ex.what());
    return 2;
}
