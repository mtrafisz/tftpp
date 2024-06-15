#include "tftp.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "nfd.hpp"
#include <iostream>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

class Popup
{
public:
    bool open;
    std::string title;
    std::string message;

    Popup() : open(false), title(""), message("") {}

    Popup ErrorPopup(const std::string& message)
    {
        Popup p;
        p.open = true;
        p.title = "Error";
        p.message = message;
        return p;
    }

    Popup InfoPopup(const std::string& message)
    {
        Popup p;
        p.open = true;
        p.title = "Info";
        p.message = message;
        return p;
    }
};

int main(void)
{
    /* Initialization */

	// Initialize Winsock
#ifdef _WIN32
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }
#endif

	// Initialize Native File Dialog
    NFD_Init();

	// Initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(390, 300, "TFTPp", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

	// Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    /* State variables */
    float progress = 0.0f;  // for progress bar
    Popup popup;
    bool disable_input = false; // Disable input while transfer is in progress

    std::vector<std::string> log;

    /* Progress callback for TFTP client */
    tftp::Client::ProgressCallback progress_callback = [&progress, &popup](tftp::Client::Progress& p)
        {
            progress = (float)p.transferred_bytes / (float)p.total_bytes;
            if (p.transferred_bytes == p.total_bytes)
            {
                progress = 0.0f;
                std::string info_msg = "Transfer completed.\nTotal bytes transferred:\n" + std::to_string(p.transferred_bytes);
                popup = popup.InfoPopup(info_msg);
            }
        };

    /* Main loop */
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
			// Input fields vars
            static char address[255] = "127.0.0.1";
            static uint16_t port = 69;
            static char local_filepath[255] = "C:\\Users\\mtrafisz\\Desktop\\test.iso";
            static char remote_filepath[255] = "debian.iso";

			// Main window
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));

            ImGui::Begin("TFTPp", nullptr, window_flags);

            ImGui::SetCursorPos(ImVec2(10, 10));

            if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Client"))
                {

                    ImGui::PushItemWidth(200.0f);
                    ImGui::InputText("##address", address, 255);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    ImGui::Text("address");
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(10, 0));
                    ImGui::SameLine();

                    ImGui::PushItemWidth(50.0f);
                    ImGui::InputScalar("##port", ImGuiDataType_U16, &port);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    ImGui::Text("port");
                    ImGui::Dummy(ImVec2(0, 0));

                    ImGui::PushItemWidth(200.0f);
                    ImGui::InputText("##local_filepath", local_filepath, 255);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    ImGui::Text("local file");
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(5, 0));
                    ImGui::SameLine();

                    ImGui::PushItemWidth(40.0f);
                    if (ImGui::Button("Browse..."))
                    {
                        nfdchar_t* outPath = nullptr;
                        nfdresult_t result = NFD_OpenDialog(&outPath, NULL, 0, NULL);

                        if (result == NFD_OKAY)
                        {
                            strncpy_s(local_filepath, outPath, 255);
                            NFD_FreePath(outPath);
                        }
                        else if (result == NFD_CANCEL)
                        {
                        }
                        else
                        {
                            popup = popup.ErrorPopup("Error opening file dialog.");
                        }
                    }
                    ImGui::PopItemWidth();
                    ImGui::Dummy(ImVec2(0, 0));

                    ImGui::PushItemWidth(200.0f);
                    ImGui::InputText("##remote_filepath", remote_filepath, 255);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    ImGui::Text("remote file");

                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 5));
                    auto window_width = ImGui::GetContentRegionMax().x;
                    auto spacing = 100.0f;
                    auto total_width = 70.0f * 2 + spacing;
                    auto start_x = (window_width - total_width) / 2;

                    ImGui::SetCursorPosX(start_x);

                    if (ImGui::Button("GET", ImVec2(70, 35)))
                    {
                        if (disable_input)
                        {
							goto skip_thread_creation;
                        }

                        std::thread tftp_action([progress_callback, &popup, &disable_input, &log]()
                            {
                                disable_input = true;

                                std::ofstream file(local_filepath, std::ios::binary);
                                std::string address_str = address + std::string(":") + std::to_string(port);
                                std::string remote_filepath_str = remote_filepath;

                                try {
                                    std::streamsize sz = tftp::Client::recv(address_str, remote_filepath, file, progress_callback, std::chrono::milliseconds(100));
                                    log.push_back("Successfully downloaded " + remote_filepath_str + " - " + std::to_string(sz) + " bytes");
                                }
                                catch (const tftp::TftpError& e) {
                                    popup = popup.ErrorPopup(e.what());
                                }
                                catch (const std::exception& e) {
                                    popup = popup.ErrorPopup(e.what());
                                }

                                file.close();
                                disable_input = false; });

                        tftp_action.detach();
                    }


                    ImGui::SameLine();
                    ImGui::SetCursorPosX(start_x + 70 + spacing);

                    if (ImGui::Button("PUT", ImVec2(70, 35)) && !disable_input)
                    {
                        if (disable_input)
                        {
							goto skip_thread_creation;
                        }

                        std::thread tftp_action([progress_callback, &popup, &disable_input, &log]()
                            {
                                disable_input = true;

                                std::ifstream file(local_filepath, std::ios::binary);
                                std::string address_str = address + std::string(":") + std::to_string(port);
                                std::string remote_filepath_str = remote_filepath;

                                try {
                                    tftp::Client::send(address_str, remote_filepath, file, progress_callback, std::chrono::milliseconds(100));
                                    // log.push_back("Successfully uploaded " + remote_filepath_str + " (size: " + std::to_string(file.tellg()) + " bytes).");
                                }
                                catch (const tftp::TftpError& e) {
                                    popup = popup.ErrorPopup(e.what());
                                }
                                catch (const std::exception& e) {
                                    popup = popup.ErrorPopup(e.what());
                                }

                                file.close();
                                disable_input = false; });

                        tftp_action.detach();
                    }

                    skip_thread_creation:
                    ImGui::PopStyleVar(2);

                    ImGui::Dummy(ImVec2(0, 10));

                    ImGui::ProgressBar(progress, ImVec2(ImGui::GetContentRegionMax().x - 20, 0));

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Server"))
                {
                    ImGui::Text("Server tab");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Log"))
                {
                    for (auto& line : log)
                    {
                        ImGui::Text(line.c_str());
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

			// Popups

            if (popup.open)
            {
                ImGui::OpenPopup(popup.title.c_str());
            }

            if (ImGui::BeginPopupModal(popup.title.c_str(), &popup.open))
            {
                ImGui::Text(popup.message.c_str());
                if (ImGui::Button("OK"))
                {
                    ImGui::CloseCurrentPopup();
                    popup.open = false;
                }
                ImGui::EndPopup();
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    NFD_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
