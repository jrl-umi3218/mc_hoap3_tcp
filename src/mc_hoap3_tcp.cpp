#include <mc_control/mc_global_controller.h>
#include <mc_rtc/version.h>
#include <mc_rtc/logging.h>
#include <mc_rtc/io_utils.h>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
namespace po = boost::program_options;
namespace ba = boost::asio;

#define TCP_INIT_PORT 2600
#define TCP_CONTROL_PORT 2200

int main(int argc, char * argv[])
{
  std::string conf_file = "";
  std::string host = "localhost";
  int port = TCP_CONTROL_PORT;
  double check_time = 4;
  double cycle_time = 2;

  po::options_description desc("MCUDPControl options");
  desc.add_options()
    ("help", "Display help message")
    ("host,h", po::value<std::string>(&host), "Connection host")
    ("check_time", po::value<double>(&check_time), "Check time")
    ("cycle_time", po::value<double>(&cycle_time), "Cycle time")
    ("port,p", po::value<int>(&port), "Connection port")
    ("conf,f", po::value<std::string>(&conf_file), "Configuration file");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 1;
  }

  if(mc_rtc::MC_RTC_VERSION != mc_rtc::version())
  {
    mc_rtc::log::error("[mc_hoap3_tcp] mc_hoap3_tcp was compiled with {} but mc_rtc is at version {}, you might face subtle issues or "
                       "unexpected crashes, please recompile mc_hoap3_tcp",
                       mc_rtc::MC_RTC_VERSION, mc_rtc::version());
  }

  mc_control::MCGlobalController::GlobalConfiguration gconfig(conf_file, nullptr);
  auto config = gconfig.config;

  auto init = [&]()
  {
    mc_rtc::log::info("[mc_hoap3_tcp] Connecting TCP initialization socket");
    // Send timestep and duration over tcp to port TCP_INIT_PORT using ba
    ba::io_service io_service;
    ba::ip::tcp::socket socket(io_service);
    ba::ip::tcp::endpoint endpoint(ba::ip::address::from_string(host), TCP_INIT_PORT);
    try
    {
    socket.connect(endpoint);
    }
    catch(boost::system::system_error & e)
    {
      mc_rtc::log::error("[mc_hoap3_tcp] Could not connect to {}:{}", host, TCP_INIT_PORT);
      return false;
    }

    double init_buffer[2] = { cycle_time, check_time };
    mc_rtc::log::info("[mc_hoap3_tcp] Sending initialization cycle_time={}, check_time={} to {}:{}", init_buffer[0], init_buffer[1], host, TCP_INIT_PORT);
    try
    {
      socket.write_some(ba::buffer(init_buffer, sizeof(init_buffer)));
    }
    catch(boost::system::system_error & e)
    {
      mc_rtc::log::error("[mc_hoap3_tcp] Could not write to {}:{}", host, TCP_INIT_PORT);
      return false;
    }
    return true;
  };

  if(!init())
  {
    mc_rtc::log::error("[mc_hoap3_tcp] Failed to initialize");
    return 1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));


  auto control = [&]()
  {
    mc_rtc::log::info("[mc_hoap3_tcp] Connecting TCP sensors/control socket");
    // Send timestep and duration over tcp to port TCP_INIT_PORT using ba
    ba::io_service io_service;
    ba::ip::tcp::socket socket(io_service);
    ba::ip::tcp::endpoint endpoint(ba::ip::address::from_string(host), port);
    try
    {
    socket.connect(endpoint);
    }
    catch(boost::system::system_error & e)
    {
      mc_rtc::log::error("[mc_hoap3_tcp] Could not connect to {}:{}", host, port);
      return false;
    }

    std::array<double, 22> control; // TODO: use same struct as in C
    std::array<double, 22> sensors; // TODO: use same struct as in C
    size_t iter = 0;
    size_t max_iter = check_time * 1000 / cycle_time;
    while(iter < max_iter)
    {
      // XXX the true logic would have us read initial sensor values before controlling.
      // TODO: Ask Philippe if this is possible. Otherwise we need to ensure that the halfsitting posture matches the one of the robot

      mc_rtc::log::info("[mc_hoap3_tcp] Iteration {}", iter);
      control.fill(iter); // XXX: just for testing

      // Send control
      mc_rtc::log::info("[mc_hoap3_tcp] Sending control to {}:{}", host, port);
      try
      {
        ba::write(socket, ba::buffer(control.data(), sizeof(control)));
      }
      catch(boost::system::system_error & e)
      {
        mc_rtc::log::error("[mc_hoap3_tcp] Could not write to {}:{}", host, port);
        return false;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      mc_rtc::log::info("[mc_hoap3_tcp] Reading sensor state from {}:{}", host, port);
      try
      {
        ba::read(socket, ba::buffer(sensors.data(), sizeof(sensors)));
        mc_rtc::log::info("Read sensor state: {}", mc_rtc::io::to_string(sensors));
      }
      catch(boost::system::system_error & e)
      {
        mc_rtc::log::error("[mc_hoap3_tcp] Could not read from {}:{}", host, port);
        return false;
      }
      iter++;
    }

    return true;
  };

  if(!control())
  {
    return 2;
  }

  return 0;
}
