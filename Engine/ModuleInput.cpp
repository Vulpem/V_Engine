#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"


#include "imGUI/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

#define MAX_KEYS 300

ModuleInput::ModuleInput(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	moduleName = "ModuleInput";

	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	delete[] keyboard;
}

// Called before render is available
bool ModuleInput::Init()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	//SDL_ShowCursor(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	dropped_file[0] = '\0';
	file_was_dropped = false;

	return ret;
}

// Called every draw update
update_status ModuleInput::PreUpdate()
{
	SDL_PumpEvents();

	file_was_dropped = false;

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1 && ignoreKeyboard == false)
		{
			if(keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if(keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_z = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i) && ignoreMouse == false)
		{
			if(mouse_buttons[i] == KEY_IDLE)
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if(mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	bool quit = false;

	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		if (ignoreMouse == false)
		{
			switch (e.type)
			{
				case SDL_MOUSEWHEEL:
				{
					mouse_z = e.wheel.y;
					break;
				}
				case SDL_MOUSEMOTION:
				{
					if (captureMouse)
					{
						if (CaptureMouse(e))
						{
							ImGui::GetIO().MousePos = ImVec2(-1, -1);
							ImGui::GetIO().MousePosPrev = ImVec2(-1, -1);
						}
					}
					mouse_x = e.motion.x;
					mouse_y = e.motion.y;

					mouse_x_motion = e.motion.xrel;
					mouse_y_motion = e.motion.yrel;
					break;
				}

				case SDL_DROPFILE:
				{
					strcpy_s(dropped_file, e.drop.file);
					SDL_free(e.drop.file);
					file_was_dropped = true;
					LOG("Dropped %s", dropped_file);
					LOG("File was detected as a %s", DroppedFileFormat().data());
					break;
				}
				case SDL_QUIT:
				{
					quit = true;
					break;
				}
				case SDL_WINDOWEVENT:
				{
					if (e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						App->OnScreenResize(e.window.data1, e.window.data2);
					}
				}
			}
		}
		ImGui_ImplSdlGL3_ProcessEvent(&e);
	}

	if (GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		captureMouse = true;
	}
	else if (GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
	{
		captureMouse = false;
	}

	if (quit == true || keyboard[SDL_SCANCODE_ESCAPE] == KEY_UP)
		return UPDATE_STOP;

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

std::string ModuleInput::DroppedFileFormat()
{
	if (file_was_dropped)
	{
		char* tmp = dropped_file;
		while (*tmp != '\0')
		{
			tmp++;
		}
		while (*tmp != '.')
		{
			tmp--;
		}
		tmp++;
		return std::string(tmp);

	}
	return std::string("Error_Reading_format");
}

bool ModuleInput::CaptureMouse(SDL_Event& e)
{
	bool ret = false;
	float2 windowSize = App->window->GetWindowSize();
	if (mouse_x + e.motion.xrel >= windowSize.x)
	{
		SDL_WarpMouseInWindow(App->window->GetWindow(), 1, e.motion.y);
		e.motion.xrel = 0;
		ret = true;
	}
	else if (mouse_x + e.motion.xrel <= 0)
	{
		SDL_WarpMouseInWindow(App->window->GetWindow(), windowSize.x - 1, e.motion.y);
		e.motion.xrel = 0;
		ret = true;
	}

	if (mouse_y + e.motion.yrel >= windowSize.y)
	{
		SDL_WarpMouseInWindow(App->window->GetWindow(), e.motion.x, 1);
		e.motion.yrel = 0;
		ret = true;
	}
	else if (mouse_y + e.motion.yrel <= 0)
	{
		SDL_WarpMouseInWindow(App->window->GetWindow(), e.motion.x, windowSize.y - 1);
		e.motion.yrel = 0;
		ret = true;
	}

	return ret;
}
