#include "nfd.hpp"
#include "imgui.h"
#include "tftp.hpp"

namespace tftpp {
    class Popup {
    public:
        bool open;
        std::string title;
        std::string message;

        enum class Type {
            None,
            Info,
            Error,
            Question
        };
        Type type;

        std::function<void()> on_yes;
        std::function<void()> on_no;

        Popup() : open(false), title(""), message("") {}

        Popup(const std::string& title, const std::string& message) : open(true), title(title), message(message), type(Type::None) {}

        void popInfo(const std::string& message) {
            this->open = true;
            this->title = "Info";
            this->message = message;
            this->type = Type::Info;
        }

        void popError(const std::string& message) {
            this->open = true;
            this->title = "Error";
            this->message = message;
            this->type = Type::Error;
        }

        void popQuestion (
            const std::string& title,
            const std::string& message,
            std::function<void()> on_yes,
            std::function<void()> on_no) {
            this->open = true;
            this->title = title;
            this->message = message;
            this->type = Type::Question;
			this->on_yes = on_yes;
			this->on_no = on_no;
        }

        void renderPopup() {
            if (open) {
                ImGui::OpenPopup(title.c_str());
                if (type != Type::Question) open = false;
            }
            
            if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped(message.c_str());

                switch (type) {
                    case Type::Info:
                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        break;
                    case Type::Error:
                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        break;
                    case Type::Question:
                        if (ImGui::Button("Yes", ImVec2(120, 0))) {
                            on_yes();
                            ImGui::CloseCurrentPopup();
                            open = false;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("No", ImVec2(120, 0))) {
                            on_no();
                            ImGui::CloseCurrentPopup();
                            open = false;
                        }
                        break;
                    default:
                        break;
                }

                ImGui::EndPopup();
            }
        }
    };

    class Client {
    private:
        float transfer_progress;
        Popup popup;
        std::thread transfer_job;

        char address_input[256] = "127.0.0.1";
		uint16_t port_input = 69;
		char local_file_input[256] = "C:\\Users\\mtrafisz\\Desktop\\test.zip";
		char remote_file_input[256] = "th.zip";

    public:
        enum class State {
            Idle,
            TransferingGet,
            TransferingPut,
            Error
        };
        State state;

        Client() {
            // initialize WSA
        #ifdef _WIN32
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
        #endif
            // initialize NFD
            NFD_Init();

            transfer_progress = 0.0f;
            state = State::Idle;
        }

        ~Client() {
            // cleanup WSA
        #ifdef _WIN32
            WSACleanup();
        #endif
            // cleanup NFD
            NFD_Quit();
        }

        void setState(State state) {
            this->state = state;
        }

        State getState() {
            return state;
        }

        void runTransferJob(Client::State desired_action);
        void renderUi();
        void renderPopup() {
            popup.renderPopup();
        }
    };
}
