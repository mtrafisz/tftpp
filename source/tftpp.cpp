#include "include/tftpp.hpp"

using namespace tftpp;

void Client::runTransferJob(Client::State desired_action) {
    if (desired_action != Client::State::TransferingGet && desired_action != Client::State::TransferingPut) {
        throw std::runtime_error("Invalid desired action");
    }

    try {
        if (this->state != Client::State::Idle) {
            throw std::runtime_error("Transfer already in progress");
        } else {
            setState(desired_action);
        }
    }
    catch (const std::exception& e) {
        this->popup.popError(e.what());
        return;
    }

    auto popup_ref = &this->popup;
    auto progress_ref = &this->transfer_progress;

    auto callback = [progress_ref, popup_ref] (tftp::Client::Progress progress) {
        *progress_ref = (float)progress.transferred_bytes / (float)progress.total_bytes;

        if (!progress.transfer_active()) {
            *progress_ref = 0.0f;
            std::string message = "Transfer complete: \n" +
                std::to_string(progress.transferred_bytes) + " bytes transferred";
            popup_ref->popInfo(message);
        }
    };

    this->transfer_job = std::thread (
        [popup_ref, action_ref = &this->state, desired_action, callback,
        address_input_str = std::string(this->address_input),
        port_input_str = std::to_string(this->port_input),
        local_file_str = std::string(this->local_file_input),
        remote_file_str = std::string(this->remote_file_input)] () {
        
        auto full_address = address_input_str + ":" + port_input_str;
        const std::chrono::milliseconds update_interval(100);

        try {
            if (desired_action == Client::State::TransferingGet) {
                bool cancel = false;

                if (std::filesystem::exists(local_file_str)) {
                    popup_ref->popQuestion (
                        "File already exists",
                        "Do you want to overwrite the file?",
                        [local_file_str] () {
                            std::filesystem::remove(local_file_str);
                        },
                        [&cancel] () {
                            cancel = true;
                        }
                    );

                    while (popup_ref->open) {
                        std::this_thread::sleep_for(update_interval);
                    }
                }

                if (cancel) {
                    throw std::runtime_error("Transfer cancelled");
                }

                std::ofstream file(local_file_str, std::ios::binary);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open local file for writing");
                }

                tftp::Client::recv(full_address, remote_file_str, file, callback, update_interval);
            } else if (desired_action == Client::State::TransferingPut) {
                if (!std::filesystem::exists(local_file_str)) {
                    throw std::runtime_error("Local file does not exist");
                }

                std::ifstream file(local_file_str, std::ios::binary);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open local file for reading");
                }

                tftp::Client::send(full_address, remote_file_str, file, callback, update_interval);
            }
        } catch (const tftp::TftpError& e) {
            popup_ref->popError(e.what());
        } catch (const std::exception& e) {
            popup_ref->popError(e.what());
        } catch (...) {
			popup_ref->popError("Unknown error");
		}

        *action_ref = Client::State::Idle;
    });

    this->transfer_job.detach();
}

void Client::renderUi() {
    ImGui::PushItemWidth(200.0f);
    ImGui::InputText("##address", address_input, sizeof(address_input));
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::Text("address");
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10.0f, 0.0f));
    ImGui::SameLine();

    ImGui::PushItemWidth(50.0f);
    ImGui::InputScalar("##port", ImGuiDataType_U16, &port_input);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::Text("port");
    
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::PushItemWidth(200.0f);
    ImGui::InputText("##local_file", local_file_input, sizeof(local_file_input));
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::Text("local file");
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(5.0f, 0.0f));
    ImGui::SameLine();

    if (ImGui::Button("Browse", ImVec2(65.0f, 0.0f))) {}

    ImGui::Dummy(ImVec2(0.0f, 0.0f));

	ImGui::PushItemWidth(200.0f);
	ImGui::InputText("##remote_file", remote_file_input, sizeof(remote_file_input));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::Text("remote file");

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    auto width_px = ImGui::GetContentRegionMax().x;
    auto spacing_px = 100.0f;
    auto button_width = 70.0f;

    auto total_width = 2 * button_width + spacing_px;
    auto start_x = (width_px - total_width) / 2.0f;

    ImGui::SetCursorPosX(start_x);

    if (ImGui::Button("Get", ImVec2(button_width, 35))) {
        runTransferJob(Client::State::TransferingGet);
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(start_x + button_width + spacing_px);
    if (ImGui::Button("Put", ImVec2(button_width, 35))) {
        runTransferJob(Client::State::TransferingPut);
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::ProgressBar(transfer_progress, ImVec2(width_px - 20.0f, 20.0f));
}
