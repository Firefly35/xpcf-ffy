project('xpcf', 'cpp', default_options : ['cpp_std=c++17'])
xpcf_version = '2.6.3'
#NOTE: BOOST_OS_LINUX_AVAILABLE is defined hardcoded : must figure out why os detection doesn't work within boost/predef ?
add_project_arguments('-DMYVERSIONSTRING="1.10.1"', 
    '-DBOOST_OS_LINUX_AVAILABLE',
    '-DBOOST_ALL_NO_LIB', 
    '-DXPCF_USE_BOOST',
    '-DXPCFVERSION="' + xpcf_version + '"', 
    language : 'cpp')


cc = meson.get_compiler('cpp')
boost_lib_dir = meson.project_source_root() / '.build/boost/install/lib'
boost_dep = declare_dependency(
  dependencies : [cc.find_library('boost_atomic', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_charconv', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_chrono', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_cobalt', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_container', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_context', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_contract', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_coroutine', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_date_time', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_fiber', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_fiber_numa', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_filesystem', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_graph', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_iostreams', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_json', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_locale', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_log', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_log_setup', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_nowide', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_test_exec_monitor', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_prg_exec_monitor', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_process', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_program_options', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_random', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_serialization', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_stacktrace_addr2line', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_stacktrace_backtrace', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_stacktrace_basic', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_stacktrace_from_exception', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_stacktrace_noop', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_thread', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_timer', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_type_erasure', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_unit_test_framework', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_url', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_wave', dirs : [boost_lib_dir]), \ 
    cc.find_library('boost_wserialization', dirs : [boost_lib_dir])], \ 
  include_directories : include_directories('.build/boost/install/include/boost')
)

incdir = include_directories('interfaces', 'libs/stduuid/include')
libxpcf = shared_library('xpcf', 'src/AliasManager.cpp', 'src/Factory.cpp', 'src/InjectableMetadata.cpp', 'src/Registry.cpp',\
    'src/tinyxml2.cpp', \
    'src/tinyxmlhelper.cpp', \
    'src/ComponentMetadata.cpp', \
    'src/InterfaceMetadata.cpp', 
    'src/MethodMetadata.cpp', \
    'src/ComponentBase.cpp', \
    'src/ComponentManager.cpp', \ 
    'src/ModuleMetadata.cpp', \
    'src/PathBuilder.cpp', \
    'src/ModuleManager.cpp', \
    'src/Enumerator.cpp', \
    'src/Property.cpp', \
    'src/PropertyMap.cpp', \
    'src/Exception.cpp', \
    'src/BaseException.cpp', \
    'src/PropertyManager.cpp', \
    'src/ConfigurableBase.cpp', \
    'src/BaseTask.cpp', \
    'src/ErrorMessage.cpp', 
    include_directories : incdir, 
    dependencies : boost_dep,
    version : xpcf_version, 
    soversion : '1')

libxpcfSampleComponent = shared_library('xpcfSampleComponent', 
    'samples/sample_component/Ibanez7ElectricGuitar.cpp',
    'samples/sample_component/TakamineFolkGuitar.cpp', 
    'samples/sample_component/Tuner.cpp',
    'samples/sample_component/xpcfSampleComponent_main.cpp',
    'samples/sample_component/HumanMusician.cpp',
    'samples/sample_component/VirtualGuitarist.cpp',
    'samples/sample_component/Guitar.cpp',
    'samples/sample_component/JacksonDinky6ElectricGuitar.cpp',
    include_directories : ['samples/sample_component', incdir], 
    dependencies : boost_dep,
    version : xpcf_version, 
    soversion : '1',
    link_with: libxpcf)

test_xpcf = executable('test_xpcf', 
    'test/xpcf_dynamic_test.cpp',
    'test/xpcf_properties_test.cpp',
    include_directories : ['test', 'samples/sample_component', incdir],
    dependencies : boost_dep,
    link_with: libxpcf)

xpcf_grpc_gen = executable('xpcf_grpc_gen',
    'tools/generators/grpc/ASTParser.cpp',
    'tools/generators/grpc/AbstractGenerator.cpp',
    'tools/generators/grpc/ClassDescriptor.cpp',
    'tools/generators/grpc/GRPCFlatBufferGenerator.cpp',
    'tools/generators/grpc/GRPCMessage.cpp',
    'tools/generators/grpc/MethodDescriptor.cpp',
    'tools/generators/grpc/ParameterDescriptor.cpp',
    'tools/generators/grpc/GRPCProtoGenerator.cpp',
    'tools/generators/grpc/MockGenerator.cpp',
    'tools/generators/grpc/ProjectGenerator.cpp',
    'tools/generators/grpc/ProxyGenerator.cpp',
    'tools/generators/grpc/RemakenPathHelper.cpp',
    'tools/generators/grpc/RemoteServiceGenerator.cpp',
    'tools/generators/grpc/ServerGenerator.cpp',
    'tools/generators/grpc/TypeDescriptor.cpp',
    'tools/generators/grpc/XpcfConfigGenerator.cpp',
    'tools/generators/grpc/xpcf_grpc_gen.cpp',
    include_directories : ['tools/generators/grpc', '', incdir],
    dependencies : boost_dep, grpc,
    link_with: libxpcf)

fs = import('fs')

xpcf_config_dir = fs.expanduser('~/.xpcf')

install_data(['test/xpcf_registry_test.xml','test/xpcf_conf_test.xml'], 
    install_dir : xpcf_config_dir )