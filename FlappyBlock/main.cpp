#include <Lunaris/utility.h>
#include <Lunaris/graphics.h>
#include <Lunaris/events.h>
#include <Lunaris/audio.h>
#include "resource.h"
#include <allegro5/allegro_native_dialog.h>

using namespace Lunaris;

const std::string texture_url = "https://media.discordapp.net/attachments/883785079984369715/891887152655384666/59894.png";
constexpr float screen_zoom_debug = 1.0f;
constexpr float dist_between_pipes = 1.25f;
constexpr float speed_accel_screen = -0.003f;// -0.0067f;
constexpr size_t num_of_pipes = 3;
constexpr float dist_shadow = 0.01f;
const color around_color = color(0,0,0);
const color bg_color = color(0.0f,0.0f,0.0f,0.6f);
const int display_speeds[] = { 0, 15, 24, 30, 45, 60, 90, 120, 144, 165, 180, 240, 360, 500, 1000, 2000 };

#define myassert(X, ERRMSG) if (!X) { cout << console::color::RED << ERRMSG; lock_console(ERRMSG); }
#define logging(A, B) { cout << console::color::BLUE << "[" << A << "] " << B;}

void lock_console(const std::string&);

int main()
{
	logging("MAIN", "Starting...");


	auto main_texture = make_hybrid<texture>();
	bool just_die = false;
	std::atomic<size_t> display_speeds_toggle = 11;

	hybrid_memory<texture> bird_textures[3]	   = { make_hybrid<texture>(),make_hybrid<texture>(),make_hybrid<texture>() };
	hybrid_memory<texture> pipe_up_texture     = make_hybrid<texture>();
	hybrid_memory<texture> pipe_dw_texture     = make_hybrid<texture>();
	hybrid_memory<texture> background_texture  = make_hybrid<texture>();
	hybrid_memory<texture> background_texture2 = make_hybrid<texture>();
	hybrid_memory<texture> foreground_texture  = make_hybrid<texture>();
	hybrid_memory<font>    main_font		   = make_hybrid<font>();

	block background_block;
	block foreground_block;
	block bird_block;
	block pipes[num_of_pipes][2]; // 3 pipes per screen * 2 (up/down)
	sprite pipes_col_only[num_of_pipes][2]; // 3 pipes per screen * 2 (up/down)
	size_t index_ahead = 0, points = 0;
	text fps_counter;

	std::vector<collisionable> collision_matrix;

	text_shadow fourcorners[4];

	display disp;
	display_event_handler dispev(disp);
	transform camera;
	keys kbkeys;
	mouse mousse(disp);
	thread movement, pipe_spawn, collision_detector;
	bool anti_doubleclick = false; // used for no false double space
	bool is_running = false;
	bool reapply_prop = false;

	sprite limits[4]; // nsew
	

	logging("MAIN", "Creating display...");
	myassert(disp.create(
		display_config()
			.set_window_title("FlappyBlock | Lunaris Edition")
			.set_use_basic_internal_event_system(true)
			.set_fullscreen(false)
			.set_extra_flags(ALLEGRO_RESIZABLE|ALLEGRO_OPENGL)//ALLEGRO_DIRECT3D_INTERNAL)
			.set_display_mode(display_options().set_width(576).set_height(1024))
			.set_economy_framerate_limit(30)
			.set_framerate_limit(display_speeds[display_speeds_toggle])
		), "Failed to create display!");

	disp.set_icon_from_icon_resource(IDI_ICON1);


	const auto speed_on_time_f = [&] {
		return speed_accel_screen + static_cast<float>(cos(al_get_time())) * 0.001f;
	};

	//myassert(fp.open("flappy_block_XXXX.png"), "Could not create temporary file!");
	//
	//logging("MAIN", "Downloading resources...");
	//{
	//	downloader down;
	//	myassert(down.get_store(texture_url, [&](const char* buf, const size_t len) { fp.write(buf, len); }), "Could not download texture file!");
	//	myassert(fp.flush(), "Could not flush temporary file");
	//}

	auto fp = make_hybrid_derived<file, memfile>(get_executable_resource_as_memfile(IDB_PNG1, (WinString)L"PNG"));
	fp->flush();

	logging("MAIN", "Loading texture...");
	myassert(main_texture->load(fp), "Failed to load texture");

	*background_texture  = main_texture->create_sub(0, 0, 144, 256);
	*background_texture2 = main_texture->create_sub(146, 0, 144, 256);
	*foreground_texture  = main_texture->create_sub(292, 0, 168, 56);
	*bird_textures[0]    = main_texture->create_sub(3, 489, 17, 17);
	*bird_textures[1]    = main_texture->create_sub(31, 489, 17, 17);
	*bird_textures[2]    = main_texture->create_sub(59, 489, 17, 17);
	*pipe_up_texture     = main_texture->create_sub(56, 323, 26, 160); // up to down
	*pipe_dw_texture     = main_texture->create_sub(84, 323, 26, 160); // down to up

	myassert(main_font->create_builtin_font(), "Could not create font!");

	myassert(!bird_textures[0]->empty(),   "Could not create sub-bitmap properly (bird_texture #0)");
	myassert(!bird_textures[1]->empty(),   "Could not create sub-bitmap properly (bird_texture #1)");
	myassert(!bird_textures[2]->empty(),   "Could not create sub-bitmap properly (bird_texture #2)");
	myassert(!pipe_up_texture->empty(),    "Could not create sub-bitmap properly (pipe_up_texture)");
	myassert(!pipe_dw_texture->empty(),    "Could not create sub-bitmap properly (pipe_dw_texture)");
	myassert(!background_texture->empty(), "Could not create sub-bitmap properly (backgroud_texture)");
	myassert(!background_texture2->empty(),"Could not create sub-bitmap properly (backgroud_texture2)");
	myassert(!foreground_texture->empty(), "Could not create sub-bitmap properly (foreground_texture)");

	background_block.texture_insert(background_texture);
	background_block.texture_insert(background_texture2);
	foreground_block.texture_insert(foreground_texture);
	for(auto& i : bird_textures) bird_block.texture_insert(i);

	logging("MAIN", "Setting up stuff...");

	camera.build_classic_fixed_proportion(disp.get_width(), disp.get_height(), 144.0f / 256.0f, screen_zoom_debug);
	camera.apply();

	dispev.hook_event_handler([&](display_event& ev) {
		switch (ev.get_type()) {
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		case static_cast<int>(display::custom_events::DISPLAY_FLAG_TOGGLE):
			ev->acknowledge_resize();
			reapply_prop = true;
			ev.post_task([&dsp = ev.get_display(), &camera]{
				camera.build_classic_fixed_proportion(dsp.get_width(), dsp.get_height(), 144.0f / 256.0f, screen_zoom_debug);
				camera.apply();
				return true;
			});
			break;
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			just_die = true;
			break;
		default:
			cout << console::color::AQUA << "EV: " << ev.get_event().type;
		}
	});

	movement.task_async([&] {

		anti_doubleclick = false;

		if (!is_running) {
			bird_block.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, 0.0f);
			bird_block.set<float>(enum_sprite_float_e::ROTATION, 0.0f);
			bird_block.set<float>(enum_sprite_float_e::POS_X, 0.0f);
			bird_block.set<float>(enum_sprite_float_e::POS_Y, 0.05f * static_cast<float>(cos(7.0 * al_get_time())));

			foreground_block.set<float>(enum_sprite_float_e::ACCEL_X, 0.0f);
			bird_block.set<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND, 4.0);
		}
		else {
			bird_block.think();
			foreground_block.think();

			const float limits_val = 0.16f;
			const float offset_fix = 0.17f;
			const float limitoff_fix = -0.14f;

			float ff = 12.0f * bird_block.get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y) - 1.2f;
			if (ff < -limits_val * static_cast<float>(ALLEGRO_PI)) ff = -limits_val * static_cast<float>(ALLEGRO_PI);
			if (ff >  limits_val * static_cast<float>(ALLEGRO_PI)) ff =  limits_val * static_cast<float>(ALLEGRO_PI);

			bird_block.set<float>(enum_sprite_float_e::ROTATION, ff);
			if (ff < -0.1f) bird_block.set<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND, 15.0);
			else bird_block.set<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND, 8.0);

			if (bird_block.get<float>(enum_sprite_float_e::POS_Y) >= 1.0f) bird_block.set<float>(enum_sprite_float_e::POS_Y, 1.0f);
			if (bird_block.get<float>(enum_sprite_float_e::POS_Y) <= -1.0f) bird_block.set<float>(enum_sprite_float_e::POS_Y, -1.0f);

			foreground_block.set<float>(enum_sprite_float_e::ACCEL_X, speed_on_time_f());
			if (float thus = foreground_block.get<float>(enum_sprite_float_e::POS_X); thus <= limitoff_fix) {
				auto diff = foreground_block.get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X) - foreground_block.get<float>(enum_sprite_float_e::POS_X);
				foreground_block.set<float>(enum_sprite_float_e::POS_X, thus + offset_fix);
				foreground_block.set<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X, thus + offset_fix + diff);
			}

			for (auto& k : limits) k.think();

			for (auto& i : pipes) {
				for (auto& k : i) {
					k.think();
				}
			}
			for (auto& i : pipes_col_only) {
				for (auto& k : i) {
					k.think();
				}
			}

			if (float& curr = pipes[index_ahead][0].get<float>(enum_sprite_float_e::POS_X); curr < 0.0f) {
				index_ahead = (index_ahead + 1) % num_of_pipes;
				++points;
			}
		}

		background_block.set<size_t>(enum_block_sizet_e::RO_DRAW_FRAME, is_running ? 0 : 1);

	}, thread::speed::INTERVAL, 1.0/24);

	pipe_spawn.task_async([&] {

		if (!is_running) {
			for (auto& i : pipes) {
				i[0].set<float>(enum_sprite_float_e::ACCEL_X, 0.0f);
				i[1].set<float>(enum_sprite_float_e::ACCEL_X, 0.0f);
			}
			for (auto& i : pipes_col_only) {
				i[0].set<float>(enum_sprite_float_e::ACCEL_X, 0.0f);
				i[1].set<float>(enum_sprite_float_e::ACCEL_X, 0.0f);
			}
		}
		else {
			for (size_t i0 = 0; i0 < std::size(pipes); i0++) {
				auto& i = pipes[i0];
				auto& i2 = pipes_col_only[i0];

				bool reshuffle = false;
				for (size_t i1 = 0; i1 < std::size(i); i1++){
					auto& k = i[i1];
					auto& k2 = i2[i1];

					k.set<float>(enum_sprite_float_e::ACCEL_X, speed_on_time_f());
					k2.set<float>(enum_sprite_float_e::ACCEL_X, speed_on_time_f());

					if (float _tmp = k.get<float>(enum_sprite_float_e::POS_X); _tmp < -1.5f) {

						k.set<float>(enum_sprite_float_e::POS_X, _tmp + 3.0f * dist_between_pipes);
						k.set<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X, _tmp + 3.0f * dist_between_pipes);

						k2.set<float>(enum_sprite_float_e::POS_X, _tmp + 3.0f * dist_between_pipes);
						k2.set<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X, _tmp + 3.0f * dist_between_pipes);

						reshuffle = true;
					}
				}
				if (reshuffle) {
					float offy = ((random() % 200) / 250.0f) - 0.6f;

					i[0].set<float>(enum_sprite_float_e::POS_Y, offy - 1.1f);
					i[1].set<float>(enum_sprite_float_e::POS_Y, offy + 1.1f);

					i2[0].set<float>(enum_sprite_float_e::POS_Y, offy - 1.13f);
					i2[1].set<float>(enum_sprite_float_e::POS_Y, offy + 1.13f);
				}
			}
		}

	},thread::speed::INTERVAL, 1.0/4);

	const auto func_on_click_fly = [&] {
		if (!is_running) { // reset
			for (size_t off = 0; off < std::size(pipes); off++) {
				auto& i = pipes[off];
				auto& i2 = pipes_col_only[off];
				float offy = ((random() % 200) / 900.0f) - 0.2f;

				i[0].set<float>(enum_sprite_float_e::POS_X, (off + 1) * dist_between_pipes);
				i[1].set<float>(enum_sprite_float_e::POS_X, (off + 1) * dist_between_pipes);
				i[0].set<float>(enum_sprite_float_e::POS_Y, offy - 1.1f);
				i[1].set<float>(enum_sprite_float_e::POS_Y, offy + 1.1f);

				i2[0].set<float>(enum_sprite_float_e::POS_X, (off + 1) * dist_between_pipes);
				i2[1].set<float>(enum_sprite_float_e::POS_X, (off + 1) * dist_between_pipes);
				i2[0].set<float>(enum_sprite_float_e::POS_Y, offy - 1.13f);
				i2[1].set<float>(enum_sprite_float_e::POS_Y, offy + 1.13f);
			}
			points = 0;
		}

		bird_block.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, -0.10f);
		anti_doubleclick = true;
		is_running = true;
	};

	kbkeys.hook_event([&](const keys::key_event& ev) {

		if (!ev.down) return;
		switch (ev.key_id) {
		case ALLEGRO_KEY_SPACE:
			func_on_click_fly();
			break;
		case ALLEGRO_KEY_F11:
		{
			disp.toggle_flag(ALLEGRO_FULLSCREEN_WINDOW);
			//reapply_prop = true;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		break;
		case ALLEGRO_KEY_X:
		{
			if (++display_speeds_toggle >= std::size(display_speeds)) display_speeds_toggle = 0;
			cout << "Toggled window speed (X) to " << display_speeds[display_speeds_toggle];
			disp.set_fps_limit(display_speeds[display_speeds_toggle]);
		}
		break;
		}
	});

	mousse.hook_event([&](const int id, const mouse::mouse_event& ev) {
		switch (id) {
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			func_on_click_fly();
			break;
		}
	});

	limits[0].set<float>(enum_sprite_float_e::SCALE_X, 3.0f);
	limits[0].set<float>(enum_sprite_float_e::POS_Y, -2.0f);
	limits[1].set<float>(enum_sprite_float_e::SCALE_X, 3.0f);
	limits[1].set<float>(enum_sprite_float_e::POS_Y, 2.0f);
	limits[2].set<float>(enum_sprite_float_e::POS_X, 4.0f);
	limits[2].set<float>(enum_sprite_float_e::SCALE_X, 3.0f);
	limits[3].set<float>(enum_sprite_float_e::POS_X, -4.0f);
	limits[3].set<float>(enum_sprite_float_e::SCALE_X, 3.0f);

	for (auto& i : limits) {
		i.set<float>(enum_sprite_float_e::SCALE_G, 2.0f);
		i.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, around_color);
		i.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	}

	background_block.set<float>(enum_sprite_float_e::SCALE_G, 2.0f);
	foreground_block.set<float>(enum_sprite_float_e::SCALE_G, 2.0f);
	foreground_block.set<float>(enum_sprite_float_e::SCALE_X, 1.2f);
	foreground_block.set<float>(enum_sprite_float_e::SCALE_Y, 0.18f);
	foreground_block.set<float>(enum_sprite_float_e::POS_Y, 0.9f);
	bird_block.set<float>(enum_sprite_float_e::SCALE_G, 0.28f);
	bird_block.set<float>(enum_sprite_float_e::SCALE_Y, 0.8f);
	bird_block.set<float>(enum_sprite_float_e::ACCEL_Y, 0.017f);
	bird_block.set<float>(enum_sprite_float_e::DRAW_MOVEMENT_RESPONSIVENESS, 2.0f);

	for(auto& i : fourcorners) i.clr = { 0,0,0 };
	fourcorners[0].offset_x =  dist_shadow;
	fourcorners[0].offset_y =  6.0f * dist_shadow;
	fourcorners[1].offset_x = -dist_shadow;
	fourcorners[1].offset_y =  6.0f * dist_shadow;
	fourcorners[2].offset_x =  dist_shadow;
	fourcorners[2].offset_y = -6.0f * dist_shadow;
	fourcorners[3].offset_x = -dist_shadow;
	fourcorners[3].offset_y = -6.0f * dist_shadow;

	fps_counter.set<float>(enum_sprite_float_e::POS_X, -0.989f);
	fps_counter.set<float>(enum_sprite_float_e::POS_Y, -0.989f);
	fps_counter.set<float>(enum_sprite_float_e::SCALE_G, 0.08f);
	fps_counter.set<float>(enum_sprite_float_e::SCALE_Y, 1.2f);
	fps_counter.set<float>(enum_sprite_float_e::THINK_ELASTIC_SPEED_PROP, 0.0f);
	fps_counter.set<float>(enum_text_float_e::DRAW_UPDATES_PER_SEC, 0.0f); // no need, little text, no real impact
	//fps_counter.set<float>(enum_text_float_e::DRAW_RESOLUTION, 0.5f);
	fps_counter.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE, true);
	fps_counter.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_NO_EFFECT_ON_SCALE, true);
	fps_counter.font_set(main_font);
	for (auto& i : fourcorners) fps_counter.shadow_insert(i);

	for (auto& k : pipes) {
		k[0].set<float>(enum_sprite_float_e::POS_X, 2.0f);
		k[0].set<float>(enum_sprite_float_e::POS_Y, -1.0f);
		k[0].set<float>(enum_sprite_float_e::SCALE_G, 1.5f);
		k[0].set<float>(enum_sprite_float_e::SCALE_X, 0.35f);
		k[0].texture_insert(pipe_up_texture);

		k[1].set<float>(enum_sprite_float_e::POS_X, 2.0f);
		k[1].set<float>(enum_sprite_float_e::POS_Y, 1.0f);
		k[1].set<float>(enum_sprite_float_e::SCALE_G, 1.5f);
		k[1].set<float>(enum_sprite_float_e::SCALE_X, 0.35f);
		k[1].texture_insert(pipe_dw_texture);
	}
	for (auto& k : pipes_col_only) {
		k[0].set<float>(enum_sprite_float_e::POS_X, 2.0f);
		k[0].set<float>(enum_sprite_float_e::POS_Y, -1.0f);
		k[0].set<float>(enum_sprite_float_e::SCALE_G, 1.5f);
		k[0].set<float>(enum_sprite_float_e::SCALE_X, 0.35f);
		//k[0].set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(0,100,100,20));
		//k[0].set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);

		k[1].set<float>(enum_sprite_float_e::POS_X, 2.0f);
		k[1].set<float>(enum_sprite_float_e::POS_Y, 1.0f);
		k[1].set<float>(enum_sprite_float_e::SCALE_G, 1.5f);
		k[1].set<float>(enum_sprite_float_e::SCALE_X, 0.35f);
		//k[1].set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(0,100,100,20));
		//k[1].set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	}
	
	fps_counter.set<safe_data<std::string>>(enum_text_safe_string_e::STRING, std::string("POINTS: ..."));

	for (auto& i : limits) collision_matrix.push_back(i);
	for (auto& i : pipes_col_only) { for (auto& j : i) collision_matrix.push_back(j); }
	collision_matrix.push_back(foreground_block);
	{
		collisionable col(bird_block);
		col.set_work([&](const collisionable::result res, sprite& src) {
			if (res.dir_to != 0) {
				if (is_running) {
					is_running = false;
					index_ahead = 0;
				}
			}
		});

		collision_matrix.push_back(std::move(col));
	}
	// do not touch collision_matrix for now on

	collision_detector.task_async([&] {
		
		work_all_auto(collision_matrix.data(), collision_matrix.data() + collision_matrix.size());

	}, thread::speed::INTERVAL, 1.0/10);
	

	logging("MAIN", "Started.");

	auto nowww = std::chrono::system_clock::now() + std::chrono::milliseconds(333);
	//size_t fps_var = 0;

	disp.set_as_target();

	while (!just_die)
	{
		//if (reapply_prop) {
		//	reapply_prop = false;
		//	camera.build_classic_fixed_proportion(disp.get_width(), disp.get_height(), 144.0f / 256.0f, screen_zoom_debug);
		//	camera.apply();
		//}

		bg_color.clear_to_this();

		background_block.draw();

		for (auto& i : pipes) {
			for (auto& j : i) j.draw();
		}
		for (auto& i : pipes_col_only) {
			for (auto& j : i) j.draw();
		}

		foreground_block.draw();


		for (auto& i : limits) i.draw();

		fps_counter.draw();
		bird_block.draw();

		disp.flip();

		if (std::chrono::system_clock::now() > nowww) {
			nowww = std::chrono::system_clock::now() + std::chrono::milliseconds(333);
			fps_counter.set<safe_data<std::string>>(enum_text_safe_string_e::STRING, /*("FPS: " + std::to_string(fps_var * 3) +*/ "POINTS: " + std::to_string(points));
			//fps_var = 0;
		}
		//else ++fps_var;
	}
	disp.destroy();

	logging("MAIN", "Closed the app.");
	return 0;
}


void lock_console(const std::string& err)
{
#ifdef _DEBUG
	while (1) {
		std::string str;
		std::getline(std::cin, str);
	}
#else
	al_show_native_message_box(nullptr, "ERROR!", "There was an error!", err.c_str(), nullptr, ALLEGRO_MESSAGEBOX_ERROR);
	std::terminate();
#endif
}