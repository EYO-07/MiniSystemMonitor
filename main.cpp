#include "pch.h"
#include "HardwareMonitorWrapper.h"
using namespace System::Windows::Forms; // Para Windows Forms
using namespace System::Drawing;
using namespace System::IO; // Para Directory e File
using namespace WMPLib; // Também necessário para IWMPPlaylist e IWMPMedia
using namespace AxWMPLib; // Para Windows Media Player
using namespace System::Diagnostics;

// Formulário principal da GUI
public ref class MainForm : public Form {
private:
    HardwareMonitorWrapper^ wrapper;
    Label^ cpuLabel;
    Label^ gpuLabel;
    Label^ diskLabel;
    Label^ ramLabel;
	Label^ motherLabel;
    System::Windows::Forms::Timer^ updateTimer;
    System::Windows::Forms::Timer^ playTimer;
    AxWindowsMediaPlayer^ mediaPlayer; // Controle para tocar MP3
    System::Collections::Generic::List<String^>^ mp3FilesList;
    System::Windows::Forms::ContextMenuStrip^ contextMenu; // Adicionado: menu de contexto
    int label_width = 260;
    int window_width = label_width ;
    double font_size = 8.5;
    int label_height = 20;
    int label_starting_X = 5;
    int label_starting_Y = 7;
	bool isYT_DLP = false;
	String^ YT_DLP_fullPath;
	ToolStripMenuItem^ downItem;
	ToolStripMenuItem^ clearListItem;
	int height_with_media, height_no_media;
public:
    MainForm() {
        wrapper = gcnew HardwareMonitorWrapper();
        InitializeComponents();
        this->Load += gcnew EventHandler(this, &MainForm::MainForm_Load); // Adicionado
        this->Activated += gcnew EventHandler(this, &MainForm::Form_Activated);   // Adicionado
        this->Deactivate += gcnew EventHandler(this, &MainForm::Form_Deactivate); // Adicionado
        this->FormClosing += gcnew FormClosingEventHandler(this, &MainForm::Form_Closing);
        mediaPlayer->PlayStateChange += gcnew AxWMPLib::_WMPOCXEvents_PlayStateChangeEventHandler(this, &MainForm::MediaPlayer_PlayStateChange);
        this->KeyDown += gcnew KeyEventHandler(this, &MainForm::Form_KeyDown); // Adicionado para captura de teclas
    }
private:    
    void InitializeComponents() {
        this->Text = "MiniSystemMonitor"; {
            this->StartPosition = FormStartPosition::Manual;
            // this->Size = System::Drawing::Size(window_width, window_height); // Tamanho da janela
            this->BackColor = Color::Black;
            this->Opacity = 0.70;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
            this->TopMost = true;
            this->KeyPreview = true;
        }
        // -- CPU 
        cpuLabel = gcnew Label(); { 
            cpuLabel->Location = Point(label_starting_X, label_starting_Y);       // Posição (x, y)
            cpuLabel->Size = System::Drawing::Size(label_width, label_height); // Tamanho do label
            cpuLabel->Text = "[ CPU ] ... Initializing ...";
            cpuLabel->ForeColor = Color::Yellow;
            cpuLabel->BackColor = Color::Black;
            cpuLabel->Font = gcnew Drawing::Font("Consolas", font_size, FontStyle::Bold);
            this->Controls->Add(cpuLabel);
        }
        // -- GPU 
        gpuLabel = gcnew Label(); {
            gpuLabel->Location = Point(label_starting_X, cpuLabel->Location.Y + label_height );       // Posição (x, y)
            gpuLabel->Size = System::Drawing::Size(label_width, label_height); // Tamanho do label
            gpuLabel->Text = "[ GPU ] ... Initializing ...";
            gpuLabel->ForeColor = Color::Yellow;
            gpuLabel->BackColor = Color::Black;
            gpuLabel->Font = gcnew Drawing::Font("Consolas", font_size, FontStyle::Bold);
            this->Controls->Add(gpuLabel);
        }
        // -- RAM
        ramLabel = gcnew Label(); {
            ramLabel->Location = Point(label_starting_X, gpuLabel->Location.Y + label_height );       // Posição (x, y)
            ramLabel->Size = System::Drawing::Size(label_width, label_height); // Tamanho do label
            ramLabel->Text = "[ Ram ] ... Initializing ...";
            ramLabel->ForeColor = Color::White;
            ramLabel->BackColor = Color::Black;
            ramLabel->Font = gcnew Drawing::Font("Consolas", font_size, FontStyle::Bold);
            this->Controls->Add(ramLabel);
        }
        // -- DISK
        diskLabel = gcnew Label(); {
            diskLabel->Location = Point(label_starting_X, ramLabel->Location.Y + label_height );       // Posição (x, y)
            diskLabel->Size = System::Drawing::Size(label_width, label_height); // Tamanho do label
            diskLabel->Text = "[ Disk ] ... Initializing ...";
            diskLabel->ForeColor = Color::White;
            diskLabel->BackColor = Color::Black;
            diskLabel->Font = gcnew Drawing::Font("Consolas", font_size, FontStyle::Bold);
            this->Controls->Add(diskLabel);
        }
        // >>
		
		motherLabel = gcnew Label(); {
			motherLabel->Location = Point(label_starting_X, diskLabel->Location.Y + label_height );       // Posição (x, y)
            motherLabel->Size = System::Drawing::Size(label_width, label_height); // Tamanho do label
            motherLabel->Text = "[ MB ] ... Initializing ...";
            motherLabel->ForeColor = Color::White;
            motherLabel->BackColor = Color::Black;
            motherLabel->Font = gcnew Drawing::Font("Consolas", font_size, FontStyle::Bold);
            this->Controls->Add(motherLabel);
		}
		
		// <<
		// --- Windows Media Player ---
        mediaPlayer = gcnew AxWindowsMediaPlayer(); {
            mediaPlayer->Location = Point(label_starting_X, motherLabel->Location.Y+label_height);
            mediaPlayer->Size = System::Drawing::Size(label_width - 10, (label_width - 10) * 9 / 16 );
            mediaPlayer->Visible = true;
            mediaPlayer->BackColor = Color::Black; // Tenta definir o fundo do controle como preto
            mediaPlayer->ForeColor = Color::White; // Tenta definir o texto como branco (pode não afetar todos os elementos)
            this->Controls->Add(mediaPlayer);
        }
        // --
		height_no_media = mediaPlayer->Location.Y;
		height_with_media = mediaPlayer->Location.Y+ mediaPlayer->Size.Height+10;
        this->ClientSize = System::Drawing::Size(window_width, height_no_media );
		InitializeContextMenu();
        // Timer para atualização automática
        updateTimer = gcnew System::Windows::Forms::Timer(); {
            updateTimer->Interval = 1000;              // 1 segundo
            updateTimer->Tick += gcnew EventHandler(this, &MainForm::update);
            updateTimer->Start();                      // Inicia o timer
        }
        // -- 
        playTimer = gcnew System::Windows::Forms::Timer(); { 
            playTimer->Interval = 100; // 100ms de atraso
            playTimer->Tick += gcnew EventHandler(this, &MainForm::PlayTimer_Tick);
            playTimer->Enabled = false; // Desativado por padrão
        }
    }
	void SFC_Click(Object^ sender, EventArgs^ e){
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = "cmd";       // Caminho do executável
			startInfo->Arguments = "/k sfc /scannow";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void DISM_Check_Click(Object^ sender, EventArgs^ e){
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = "cmd";       // Caminho do executável
			startInfo->Arguments = "/k DISM /Online /Cleanup-Image /CheckHealth";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			//startInfo->StandardOutputEncoding = System::Text::Encoding::GetEncoding(GetOEMCP());
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void DISM_Scan_Click(Object^ sender, EventArgs^ e){
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = "cmd";       // Caminho do executável
			startInfo->Arguments = "/k DISM /Online /Cleanup-Image /ScanHealth";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			//startInfo->StandardOutputEncoding = System::Text::Encoding::GetEncoding(GetOEMCP());
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void DISM_Restore_Click(Object^ sender, EventArgs^ e){
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = "cmd";       // Caminho do executável
			startInfo->Arguments = "/k DISM /Online /Cleanup-Image /RestoreHealth";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			//startInfo->StandardOutputEncoding = System::Text::Encoding::GetEncoding(GetOEMCP());
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void winget_click(Object^ sender, EventArgs^ e){
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = "cmd";       // Caminho do executável
			startInfo->Arguments = "/k winget upgrade --all";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			//startInfo->StandardOutputEncoding = System::Text::Encoding::GetEncoding(GetOEMCP());
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void InitializeContextMenu(){
        contextMenu = gcnew System::Windows::Forms::ContextMenuStrip(); {
			ToolStripMenuItem^ systemSubmenuItem = gcnew ToolStripMenuItem("System Tools");
			{
				ToolStripMenuItem^ option1Item = gcnew ToolStripMenuItem("sfc /scannow"); {
					option1Item->Click += gcnew EventHandler(this, &MainForm::SFC_Click);
					systemSubmenuItem->DropDownItems->Add(option1Item);
				}
				ToolStripMenuItem^ option2Item = gcnew ToolStripMenuItem("Dism ... /CheckHealth"); {
					option2Item->Click += gcnew EventHandler(this, &MainForm::DISM_Check_Click);
					systemSubmenuItem->DropDownItems->Add(option2Item);
				}
				ToolStripMenuItem^ option3Item = gcnew ToolStripMenuItem("Dism ... /ScanHealth"); {
					option3Item->Click += gcnew EventHandler(this, &MainForm::DISM_Scan_Click);
					systemSubmenuItem->DropDownItems->Add(option3Item);
				}
				ToolStripMenuItem^ option4Item = gcnew ToolStripMenuItem("Dism ... /RestoreHealth"); {
					option4Item->Click += gcnew EventHandler(this, &MainForm::DISM_Restore_Click);
					systemSubmenuItem->DropDownItems->Add(option4Item);
				}
				ToolStripMenuItem^ option5Item = gcnew ToolStripMenuItem("winget upgrade --all"); {
					option5Item->Click += gcnew EventHandler(this, &MainForm::winget_click);
					systemSubmenuItem->DropDownItems->Add(option5Item);
				}
			}
			contextMenu->Items->Add(systemSubmenuItem);
            ToolStripMenuItem^ loadFileItem = gcnew ToolStripMenuItem("Load Media File"); {
                loadFileItem->Click += gcnew EventHandler(this, &MainForm::LoadFile_Click);
                contextMenu->Items->Add(loadFileItem);
            }
            ToolStripMenuItem^ PlayPauseItem = gcnew ToolStripMenuItem("Play/Pause"); {
                PlayPauseItem->Click += gcnew EventHandler(this, &MainForm::PlayPause_Click);
                contextMenu->Items->Add(PlayPauseItem);
            }
            ToolStripMenuItem^ volumeItem = gcnew ToolStripMenuItem("Volume"); {
                volumeItem->Click += gcnew EventHandler(this, &MainForm::VolumeItem_Click);
                contextMenu->Items->Add(volumeItem);
            }
			String^ exeDir = System::IO::Path::GetDirectoryName(Application::ExecutablePath);
			if(!YT_DLP_fullPath) YT_DLP_fullPath = System::IO::Path::Combine(exeDir, "yt-dlp.exe"); 
			isYT_DLP = File::Exists(YT_DLP_fullPath);
			downItem = gcnew ToolStripMenuItem("Download Video(URL)"); 
			if(isYT_DLP){ downItem->Text = "Dowload Video (URL)"; } else {
				downItem->Text = "set yt-dlp path";
			}
			downItem->Click += gcnew EventHandler(this, &MainForm::YT_DLP_Click);
			contextMenu->Items->Add(downItem);
			ToolStripMenuItem^ opacityItem = gcnew ToolStripMenuItem("Transparency"); {
                opacityItem->Click += gcnew EventHandler(this, &MainForm::OpacityItem_Click);
                contextMenu->Items->Add(opacityItem);
            }
            clearListItem = gcnew ToolStripMenuItem("Clear List"); {
                clearListItem->Click += gcnew EventHandler(this, &MainForm::ClearList_Click);
                contextMenu->Items->Add(clearListItem);
            }
        } 
        this->ContextMenuStrip = contextMenu; 
	}
    void update(Object^ sender, EventArgs^ e) {
        wrapper->update();
        double load = 0.0;
        double temp = 0.0;
        double fan = 0.0;
        // -- CPU
        {
            load = wrapper->cpu_load;
            temp = wrapper->cpu_temp;
			if (temp < 38){
				cpuLabel->ForeColor = Color::LightBlue; 
			}
            else if (temp < 40) { 
				cpuLabel->ForeColor = Color::White; 
			}
            else if (temp < 50) {
                cpuLabel->ForeColor = Color::Yellow;
            }
            else if (temp < 60) {
                cpuLabel->ForeColor = Color::Orange;
            }
            else {
                cpuLabel->ForeColor = Color::Red;
            }
            fan = wrapper->cpu_fan;
            if (load + temp + fan) cpuLabel->Text = "[ CPU ] ";
            if (load) cpuLabel->Text += load.ToString("F1") + "% ";
            if (temp) cpuLabel->Text += temp.ToString("F1") + "°C ";
            if (fan) cpuLabel->Text += fan.ToString("F1") + " RPM ";
        }
        // -- GPU
        {
            load = wrapper->gpu_load;
            temp = wrapper->gpu_temp;
			if (temp < 39) {
				gpuLabel->ForeColor = Color::LightBlue; 
			}
            else if (temp < 40) { 
				gpuLabel->ForeColor = Color::White; 
			}
            else if (temp < 50) {
                gpuLabel->ForeColor = Color::Yellow;
            }
            else if (temp < 60) {
                gpuLabel->ForeColor = Color::Orange;
            }
            else {
                gpuLabel->ForeColor = Color::Red;
            }
            fan = wrapper->gpu_fan;
            if (load + temp + fan) gpuLabel->Text = "[ GPU ] ";
            if (load) gpuLabel->Text += load.ToString("F1") + "% ";
            if (temp) gpuLabel->Text += temp.ToString("F1") + "°C ";
            if (fan) gpuLabel->Text += fan.ToString("F1") + " RPM ";
        }
        // -- Ram
        {
            load = wrapper->ram_load;
			if (load < 20){
				ramLabel->ForeColor = Color::LightBlue; 
			}
            else if (load < 30) { 
				ramLabel->ForeColor = Color::White; 
			}
            else if (load < 40) {
                ramLabel->ForeColor = Color::Yellow;
            }
            else if (load < 50) {
                ramLabel->ForeColor = Color::Orange;
            }
            else {
                ramLabel->ForeColor = Color::Red;
            }
            temp = wrapper->ram_temp;
            fan = wrapper->ram_fan;
            if (load + temp + fan) ramLabel->Text = "[ RAM ] ";
            if (load) ramLabel->Text += load.ToString("F1") + "% ";
            if (temp) ramLabel->Text += temp.ToString("F1") + "°C ";
            if (fan) ramLabel->Text += fan.ToString("F1") + " RPM ";
        }
        // -- Disk
        {
            load = wrapper->drive_load;
            temp = wrapper->drive_temp;
			if (temp < 32){
				diskLabel->ForeColor = Color::LightBlue; 
			}
            else if (temp < 34) { 
				diskLabel->ForeColor = Color::White; 
			}
            else if (temp < 36) {
                diskLabel->ForeColor = Color::Yellow;
            }
            else if (temp < 37) {
                diskLabel->ForeColor = Color::Orange;
            }
            else {
                diskLabel->ForeColor = Color::Red;
            }
            fan = wrapper->drive_fan;
            if (load + temp + fan) diskLabel->Text = "[ Disk ] ";
            if (0) diskLabel->Text += load.ToString("F1") + "% ";
            if (temp) diskLabel->Text += temp.ToString("F1") + "°C ";
            if (fan) diskLabel->Text += fan.ToString("F1") + " RPM ";
        }
		// >>
		{
			/*
            load = wrapper->mother_load;
            temp = wrapper->mother_temp;
			if (temp < 32){
				motherLabel->ForeColor = Color::LightBlue; 
			}
            else if (temp < 34) { 
				motherLabel->ForeColor = Color::White; 
			}
            else if (temp < 36) {
                motherLabel->ForeColor = Color::Yellow;
            }
            else if (temp < 37) {
                motherLabel->ForeColor = Color::Orange;
            }
            else {
                motherLabel->ForeColor = Color::Red;
            }
            fan = wrapper->drive_fan;
			*/
			motherLabel->Text = wrapper->fanInfo;
			/*
            if (load + temp + fan) motherLabel->Text = "[ MB ] ";
            if (load) motherLabel->Text += load.ToString("F1") + "% ";
            if (temp) motherLabel->Text += temp.ToString("F1") + "°C ";
            if (fan) motherLabel->Text += fan.ToString("F1") + " RPM ";
			*/
        }
		// <<
    }
    void MainForm_Load(Object^ sender, EventArgs^ e) {
        String^ exeDir = Path::GetDirectoryName(Application::ExecutablePath);
        System::Collections::Generic::IEnumerable<String^>^ mp3FilesEnumerable = Directory::EnumerateFiles(exeDir, "*.mp3", SearchOption::AllDirectories);
        // Converte o IEnumerable para uma List para permitir embaralhamento
        mp3FilesList = gcnew System::Collections::Generic::List<String^>(mp3FilesEnumerable);
        if (mp3FilesList->Count == 0) {
            //MessageBox::Show("Nenhum arquivo MP3 encontrado no diretório do executável ou subdiretórios.", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            //return;
        }
        else {
            // Embaralha a lista usando o algoritmo Fisher-Yates
            Random^ rand = gcnew Random();
            for (int i = mp3FilesList->Count - 1; i > 0; i--) {
                int j = rand->Next(0, i + 1);
                String^ temp = mp3FilesList[i];
                mp3FilesList[i] = mp3FilesList[j];
                mp3FilesList[j] = temp;
            }
            // Toca o primeiro arquivo diretamente
            mediaPlayer->URL = mp3FilesList[0];
            mediaPlayer->Ctlcontrols->play();
        }
        mediaPlayer->uiMode = "none";
    }
    void Form_Activated(Object^ sender, EventArgs^ e) {
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
    }
    void Form_Deactivate(Object^ sender, EventArgs^ e) {
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
    }
    void Form_Closing(Object^ sender, FormClosingEventArgs^ e) {
        if (mediaPlayer != nullptr && mediaPlayer->currentPlaylist != nullptr) {
            mediaPlayer->Ctlcontrols->stop(); // Limpa a playlist
        }
    }
    void MediaPlayer_PlayStateChange(Object^ sender, AxWMPLib::_WMPOCXEvents_PlayStateChangeEvent^ e) {
        WMPPlayState currentState = static_cast<WMPPlayState>(e->newState);
        if (currentState == WMPPlayState::wmppsMediaEnded) {
            if (mp3FilesList->Count > 0) {
                Random^ rand = gcnew Random();
                int nextIndex = rand->Next(0, mp3FilesList->Count);
                mediaPlayer->URL = mp3FilesList[nextIndex];
                playTimer->Enabled = true; // Inicia o timer para chamar play()
            }
        }
		// -- 
		if (mediaPlayer->playState == WMPPlayState::wmppsPlaying) { 
            this->ClientSize = System::Drawing::Size(window_width, height_with_media);
		} else {
            this->ClientSize = System::Drawing::Size(window_width, height_no_media);
		}
    }
    void Form_KeyDown(Object^ sender, KeyEventArgs^ e) {
        if (e->KeyCode == Keys::Space) { 
            WMPPlayState currentState = static_cast<WMPPlayState>(mediaPlayer->playState);
            if (currentState == WMPPlayState::wmppsPlaying) {
                mediaPlayer->Ctlcontrols->pause(); // Pausa se estiver tocando
            }
            else if (currentState == WMPPlayState::wmppsPaused || currentState == WMPPlayState::wmppsStopped) {
                mediaPlayer->Ctlcontrols->play(); // Toca se estiver pausado ou parado
            }
            e->Handled = true; // Marca o evento como tratado para evitar propagação
        }
    }
    void LoadFile_Click(Object^ sender, EventArgs^ e) {
        System::Windows::Forms::OpenFileDialog^ openFileDialog = gcnew System::Windows::Forms::OpenFileDialog();
        openFileDialog->Filter = "MP3/MP4|*.mp3;*.mp4|Any (*.*)|*.*";
        openFileDialog->Title = "Selecione um arquivo";
        openFileDialog->Multiselect = false;
        if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
            String^ selectedFile = openFileDialog->FileName;
            mediaPlayer->URL = selectedFile;
            mediaPlayer->Ctlcontrols->play();
            mp3FilesList->Add(selectedFile); // Opcional
            mediaPlayer->Size = System::Drawing::Size(label_width - 10, (label_width - 10)*9/16 ); 
            this->ClientSize = System::Drawing::Size(window_width, height_with_media );
			clearListItem->Text = "Clear List ("+mp3FilesList->Count.ToString("F0")+")";
        }
    }
    void ClearList_Click(Object^ sender, EventArgs^ e) { 
		mp3FilesList->Clear(); 
		clearListItem->Text = "Clear List ("+mp3FilesList->Count.ToString("F0")+")";
	}
    void PlayTimer_Tick(Object^ sender, EventArgs^ e) {
        playTimer->Enabled = false; // Desativa o timer após um uso
        mediaPlayer->Ctlcontrols->play(); // Tenta iniciar a reprodução
    }
    void OpacitySlider_Scroll(Object^ sender, EventArgs^ e) {
        TrackBar^ slider = safe_cast<TrackBar^>(sender);
        this->Opacity = slider->Value / 100.0;
    }
    void OpacityItem_Click(Object^ sender, EventArgs^ e) {
        Form^ popup = gcnew Form(); {
			popup->Text = "Transparency";
			popup->Size = System::Drawing::Size(300, 100);
			popup->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			popup->StartPosition = FormStartPosition::CenterScreen;
			popup->MaximizeBox = false;
			popup->MinimizeBox = false;
			popup->ShowInTaskbar = false;
		}
        TrackBar^ opacitySlider = gcnew TrackBar(); {
			opacitySlider->Location = Point(10, 10);
			opacitySlider->Size = System::Drawing::Size(260, 45);
			opacitySlider->Minimum = 0;
			opacitySlider->Maximum = 100;
			opacitySlider->Value = (int)(this->Opacity * 100);
			opacitySlider->TickFrequency = 10;
			opacitySlider->Scroll += gcnew EventHandler(this, &MainForm::OpacitySlider_Scroll); 
		}
        popup->Controls->Add(opacitySlider);
        popup->ShowDialog();
    }
    void PlayPause_Click(Object^ sender, EventArgs^ e) {
        bool isPaused = (static_cast<WMPPlayState>(mediaPlayer->playState) == WMPPlayState::wmppsPaused);
        if (isPaused) { mediaPlayer->Ctlcontrols->play(); }
        else {
            mediaPlayer->Ctlcontrols->pause();
        }
    }
	void VolumeItem_Click(Object^ sender, EventArgs^ e){
		Form^ popup = gcnew Form(); {
			popup->Text = "Volume";
			popup->Size = System::Drawing::Size(300, 100);
			popup->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			popup->StartPosition = FormStartPosition::CenterScreen;
			popup->MaximizeBox = false;
			popup->MinimizeBox = false;
			popup->ShowInTaskbar = false;
		}
        TrackBar^ Slider = gcnew TrackBar(); {
			Slider->Location = Point(10, 10);
			Slider->Size = System::Drawing::Size(260, 45);
			Slider->Minimum = 0;
			Slider->Maximum = 100;
			Slider->Value = (int)(this->mediaPlayer->settings->volume);
			Slider->TickFrequency = 3;
			Slider->Scroll += gcnew EventHandler(this, &MainForm::VolumeSlider_Scroll);
		}
        popup->Controls->Add(Slider);
        popup->ShowDialog();
	}
	void VolumeSlider_Scroll(Object^ sender, EventArgs^ e){
		TrackBar^ slider = safe_cast<TrackBar^>(sender);
        this->mediaPlayer->settings->volume = slider->Value;
	}
	void YT_DLP(String^ URL) {
		Process^ process = gcnew Process();
		try {
			ProcessStartInfo^ startInfo = gcnew ProcessStartInfo();
			startInfo->FileName = YT_DLP_fullPath;       // Caminho do executável
			startInfo->Arguments = URL+" --no-playlist --progress --audio-quality 0";        // Argumentos (opcional)
			startInfo->UseShellExecute = true;      // Não usa o shell (permite redirecionar saída, se necessário)
			startInfo->RedirectStandardOutput = false; // Redireciona a saída para capturá-la (opcional)
			startInfo->CreateNoWindow = false;        // Não cria uma janela visível (opcional)
			process->StartInfo = startInfo;
			process->Start(); // Inicia o programa
		}
		catch (Exception^ ex) {
			MessageBox::Show("Aplicativo não encontrado", "Erro", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
		}
	}
	void YT_DLP_Click(Object^ sender, EventArgs^ e){
		if(isYT_DLP){
			Form^ popup = gcnew Form(); {
				popup->Text = "Dowload Video URL";
				popup->Size = System::Drawing::Size(600, 130);
				popup->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
				popup->StartPosition = FormStartPosition::CenterScreen;
				popup->MaximizeBox = false;
				popup->MinimizeBox = false;
				popup->ShowInTaskbar = false;
			}
			Label^ warn = gcnew Label(); {
				warn->Location = Point(10, 10);       // Posição (x, y)
				warn->Size = System::Drawing::Size(560, 20); // Tamanho do label
				warn->Text = "the downloaded .mp4 file will be located on MiniSystemMonitor.exe directory";
				warn->Font = gcnew Drawing::Font("Consolas", 8, FontStyle::Bold);
				popup->Controls->Add(warn);
			}
			TextBox^ textBox = gcnew System::Windows::Forms::TextBox(); {
				textBox->Location = Point(10, 10+20);
				textBox->Size = System::Drawing::Size(560, 20); 
				popup->Controls->Add(textBox);
			} 
			Button^ okButton = gcnew System::Windows::Forms::Button(); {
				okButton->Text = "OK";
				okButton->Location = Point(110, 40+20);
				okButton->Size = System::Drawing::Size(75, 25);
				okButton->Click += gcnew EventHandler(this, &MainForm::YT_DLP_okClick);
				popup->Controls->Add(okButton);
				popup->AcceptButton = okButton;
			} 
			popup->ShowDialog();
		}
		else {
			YT_DLP_fullPath = GetApplicationPathWithDialog();
			if( System::IO::Path::GetFileName(YT_DLP_fullPath)->Equals("yt-dlp.exe") ){ 				
				downItem->Text = "Dowload Video (URL)";
				isYT_DLP = true;
			} else {
				downItem->Text = "set yt-dlp path";
				isYT_DLP = false;
			}
		}
	}
	void YT_DLP_okClick(Object^ sender, EventArgs^ e){
        Button^ okButton = safe_cast<Button^>(sender);
        Form^ popup = safe_cast<Form^>(okButton->Parent);
		String^ url = popup->Controls[1]->Text; // Armazena o texto inserido
        YT_DLP(url);
        this->DialogResult = System::Windows::Forms::DialogResult::OK; // Define o resultado como OK
        popup->Close();
	}
	void YT_DLP_setPath_Click(Object^ sender, EventArgs^ e){
		
	}
	String^ GetApplicationPathWithDialog() {
        OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
        openFileDialog->Filter = "Executáveis (*.exe)|*.exe|Todos os arquivos (*.*)|*.*"; // Filtro para .exe
        openFileDialog->Title = "Selecione um Aplicativo";
        openFileDialog->Multiselect = false; // Apenas um arquivo por vez
        if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
            return openFileDialog->FileName; // Retorna o caminho completo do arquivo selecionado
        }
        return nullptr; // Retorna nullptr se o usuário cancelar
    }
};

[STAThread] // Necessário para Windows Forms
int WinMain() {
    Application::EnableVisualStyles();           // Estilo visual moderno
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew MainForm());          // Inicia o formulário
    return 0;
}


















