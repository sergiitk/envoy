#include "library/common/engine_common.h"

#include "source/common/common/random_generator.h"
#include "source/common/runtime/runtime_impl.h"

#if !defined(ENVOY_ENABLE_FULL_PROTOS)

#include "bazel/cc_proto_descriptor_library/file_descriptor_info.h"
#include "source/common/protobuf/protobuf.h"

#include "library/common/extensions/cert_validator/platform_bridge/platform_bridge_descriptor.pb.h"
#include "library/common/extensions/filters/http/local_error/filter_descriptor.pb.h"
#include "library/common/extensions/filters/http/network_configuration/filter_descriptor.pb.h"
#include "library/common/extensions/filters/http/platform_bridge/filter_descriptor.pb.h"
#include "library/common/extensions/filters/http/socket_tag/filter_descriptor.pb.h"
#include "library/common/extensions/key_value/platform/platform_descriptor.pb.h"
#include "library/common/extensions/retry/options/network_configuration/predicate_descriptor.pb.h"

namespace Envoy {

bool initialize() {
  std::vector<FileDescriptorInfo> file_descriptors = {
      protobuf::reflection::
          library_common_extensions_cert_validator_platform_bridge_platform_bridge::
              kFileDescriptorInfo,
      protobuf::reflection::library_common_extensions_filters_http_local_error_filter::
          kFileDescriptorInfo,
      protobuf::reflection::library_common_extensions_filters_http_network_configuration_filter::
          kFileDescriptorInfo,
      protobuf::reflection::library_common_extensions_filters_http_platform_bridge_filter::
          kFileDescriptorInfo,
      protobuf::reflection::library_common_extensions_filters_http_socket_tag_filter::
          kFileDescriptorInfo,
      protobuf::reflection::library_common_extensions_key_value_platform_platform::
          kFileDescriptorInfo,
      protobuf::reflection::
          library_common_extensions_retry_options_network_configuration_predicate::
              kFileDescriptorInfo,
  };
  for (const FileDescriptorInfo& descriptor : file_descriptors) {
    loadFileDescriptors(descriptor);
  }
  return true;
}

void registerMobileProtoDescriptors() {
  static bool initialized = initialize();
  (void)initialized;
}

} // namespace Envoy

#endif

namespace Envoy {

EngineCommon::EngineCommon(std::unique_ptr<Envoy::OptionsImpl>&& options)
    : options_(std::move(options)) {

#if !defined(ENVOY_ENABLE_FULL_PROTOS)
  registerMobileProtoDescriptors();
#endif

  base_ = std::make_unique<StrippedMainBase>(
      *options_, real_time_system_, default_listener_hooks_, prod_component_factory_,
      std::make_unique<PlatformImpl>(), std::make_unique<Random::RandomGeneratorImpl>(), nullptr);

  // Disabling signal handling in the options makes it so that the server's event dispatcher _does
  // not_ listen for termination signals such as SIGTERM, SIGINT, etc
  // (https://github.com/envoyproxy/envoy/blob/048f4231310fbbead0cbe03d43ffb4307fff0517/source/server/server.cc#L519).
  // Previous crashes in iOS were experienced due to early event loop exit as described in
  // https://github.com/envoyproxy/envoy-mobile/issues/831. Ignoring termination signals makes it
  // more likely that the event loop will only exit due to Engine destruction
  // https://github.com/envoyproxy/envoy-mobile/blob/a72a51e64543882ea05fba3c76178b5784d39cdc/library/common/engine.cc#L105.
  options_->setSignalHandling(false);
}

} // namespace Envoy
