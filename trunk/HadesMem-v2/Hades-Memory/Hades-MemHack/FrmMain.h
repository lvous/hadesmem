#pragma once

namespace HadesMemHack {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
  using namespace System::Diagnostics;
  using namespace System::Collections::Generic;

	/// <summary>
	/// Summary for FrmMain
	/// </summary>
	public ref class FrmMain : public System::Windows::Forms::Form
	{
	public:
		FrmMain(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~FrmMain()
		{
			if (components)
			{
				delete components;
			}
		}
  private: System::Windows::Forms::ListBox^  LstProcs;
  private: System::Windows::Forms::Button^  BtnRefresh;
  private: System::Windows::Forms::Button^  BtnOpen;
  protected: 

  protected: 



	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
      this->LstProcs = (gcnew System::Windows::Forms::ListBox());
      this->BtnRefresh = (gcnew System::Windows::Forms::Button());
      this->BtnOpen = (gcnew System::Windows::Forms::Button());
      this->SuspendLayout();
      // 
      // LstProcs
      // 
      this->LstProcs->FormattingEnabled = true;
      this->LstProcs->Location = System::Drawing::Point(12, 12);
      this->LstProcs->Name = L"LstProcs";
      this->LstProcs->Size = System::Drawing::Size(156, 446);
      this->LstProcs->Sorted = true;
      this->LstProcs->TabIndex = 0;
      // 
      // BtnRefresh
      // 
      this->BtnRefresh->Location = System::Drawing::Point(12, 464);
      this->BtnRefresh->Name = L"BtnRefresh";
      this->BtnRefresh->Size = System::Drawing::Size(75, 23);
      this->BtnRefresh->TabIndex = 1;
      this->BtnRefresh->Text = L"Refresh";
      this->BtnRefresh->UseVisualStyleBackColor = true;
      this->BtnRefresh->Click += gcnew System::EventHandler(this, &FrmMain::BtnRefresh_Click);
      // 
      // BtnOpen
      // 
      this->BtnOpen->Location = System::Drawing::Point(93, 464);
      this->BtnOpen->Name = L"BtnOpen";
      this->BtnOpen->Size = System::Drawing::Size(75, 23);
      this->BtnOpen->TabIndex = 2;
      this->BtnOpen->Text = L"Open";
      this->BtnOpen->UseVisualStyleBackColor = true;
      // 
      // FrmMain
      // 
      this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      this->ClientSize = System::Drawing::Size(475, 499);
      this->Controls->Add(this->BtnOpen);
      this->Controls->Add(this->BtnRefresh);
      this->Controls->Add(this->LstProcs);
      this->Name = L"FrmMain";
      this->Text = L"Hades-MemHack";
      this->ResumeLayout(false);

    }
#pragma endregion
  private:

    System::Void BtnRefresh_Click(System::Object^ /*sender*/, System::EventArgs^ /*e*/) 
    {
      LstProcs->Items->Clear();
      array<Process^>^ processlist = Process::GetProcesses();
      for each(Process^ current in processlist)
      {
        String^ MyString = String::Format("{0} -> {1}", current->ProcessName, current->Id);
        LstProcs->Items->Add(MyString);
      }
    }
  };

}

