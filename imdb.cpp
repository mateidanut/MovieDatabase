// Copyright 2017 Paduraru Razvan Stefan si Matei Gabriel Danut
#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include "include/imdb.h"

IMDb::IMDb() {
    // initialize what you need here.
}

IMDb::~IMDb() {}

void IMDb::add_movie(std::string movie_name,
                     std::string movie_id,
                     int timestamp,  // unix timestamp when movie was launched
                     std::vector<std::string> categories,
                     std::string director_name,
                     std::vector<std::string> actor_ids) {
    film x;
    x.movie_name = movie_name;
    x.movie_id = movie_id;
    x.timestamp = timestamp;
    x.categories = categories;
    x.director_name = director_name;
    x.actors_ids = actor_ids;

    if (IMDb::Directors.find(director_name) == IMDb::Directors.end()) {
        director R;
        R.name = director_name;
            for (auto it = actor_ids.begin(); it != actor_ids.end(); it++) {
                R.actors_ids.insert(*it);
            }
        IMDb::Directors[director_name] = R;
    } else {
        for (auto it = actor_ids.begin(); it != actor_ids.end(); it++) {
            if (IMDb::Directors[director_name].actors_ids.find(*it) ==
                IMDb::Directors[director_name].actors_ids.end()) {
                IMDb::Directors[director_name].actors_ids.insert(*it);
            }
        }
    }

    IMDb::Films[movie_id] = x;

    actorMovies y;
    for (auto it = actor_ids.begin(); it != actor_ids.end(); ++it) {
        if (IMDb::ActorMovies.find(*it) == IMDb::ActorMovies.end()) {
            IMDb::ActorMovies[*it] = y;
        }
        IMDb::ActorMovies[*it].AddMovie(timestamp);
    }

    filmRatings z;
    if (IMDb::FilmRatings.find(movie_id) == IMDb::FilmRatings.end()) {
        IMDb::FilmRatings[movie_id] = z;
    }

    movieTimeline a;
    a.timestamp = timestamp;
    a.movie_id = movie_id;
    mT.push_back(a);

    for (auto it1 = actor_ids.begin(); it1 != actor_ids.end(); ++it1) {
        for (auto it2 = it1 + 1; it2 != actor_ids.end(); ++it2) {
            Colleagues[*it1].insert(*it2);
            Colleagues[*it2].insert(*it1);

            actorPair Pair(*it1, *it2);
            if (Coops.find(Pair) == Coops.end()) {
                Coops[Pair] = 1;
            } else {
                Coops[Pair]++;
            }
        }
    }

    movieReviews b;
    b.rating_number = 0;
    b.movie_id = movie_id;
    mR.push_back(b);

    Movie_Rating_Date m;
    m.movie_id = movie_id;
    m.rating = 0;
    m.timestamp = timestamp;
	IMDb::Movie_AVG[timestamp] = m;
}

void IMDb::add_user(std::string user_id, std::string name) {
    user x;
    x.user_id = user_id;
    x.name = name;

    IMDb::Users[user_id] = x;
}

void IMDb::add_actor(std::string actor_id, std::string name) {
    actor x;
    x.actor_id = actor_id;
    x.name = name;
    IMDb::Actors[actor_id] = x;

    actorMovies y;
    if (IMDb::ActorMovies.find(actor_id) == IMDb::ActorMovies.end()) {
        IMDb::ActorMovies[actor_id] = y;
    }
}

void IMDb::add_rating(std::string user_id, std::string movie_id, int rating) {
    UserMoviePair x;
    x.user_id = user_id;
    x.movie_id = movie_id;
    IMDb::RatingsUM[x] = rating;
    double film_rating = IMDb::FilmRatings[movie_id].OverallRating();
    IMDb::FilmRatings[movie_id].AddRating(rating);

    double new_film_rating = IMDb::FilmRatings[movie_id].OverallRating();

    int timestamp = IMDb::Films[movie_id].timestamp;
    category y;

    std::vector<std::string> categs = IMDb::Films[movie_id].categories;
    for (auto it = categs.begin(); it != categs.end(); ++it) {
        if (IMDb::Categories.find(*it) == IMDb::Categories.end()) {
            IMDb::Categories[*it] = y;
        }
        if (film_rating != -1) {
            IMDb::Categories[(*it)].RemoveRating(timestamp, film_rating);
        }
        IMDb::Categories[(*it)].AddRating(timestamp, new_film_rating);
    }

    for (auto it = mR.begin(); it != mR.end(); ++it) {
        if (it->movie_id == movie_id) {
            it->rating_number = IMDb::FilmRatings[movie_id].rating_number;
        }
    }

    for (auto it = IMDb::Movie_AVG.begin(); it != IMDb::Movie_AVG.end(); ++it) {
        if (it->second.movie_id == movie_id) {
            it->second.rating = new_film_rating;
        }
    }
}

void IMDb::update_rating(std::string user_id,
                         std::string movie_id, int rating) {
    UserMoviePair x;
    x.user_id = user_id;
    x.movie_id = movie_id;

    int old_rating = IMDb::RatingsUM[x];
    IMDb::RatingsUM[x] = rating;

    double old_film_rating = IMDb::FilmRatings[movie_id].OverallRating();
    IMDb::FilmRatings[movie_id].UpdateRating(old_rating, rating);
    double film_rating = IMDb::FilmRatings[movie_id].OverallRating();

    int timestamp = IMDb::Films[movie_id].timestamp;

    std::vector<std::string> categs = IMDb::Films[movie_id].categories;
    for (auto it = categs.begin(); it != categs.end(); ++it) {
        IMDb::Categories[(*it)].UpdateRating(timestamp,
                         old_film_rating, film_rating);
    }

    for (auto it = IMDb::Movie_AVG.begin(); it != IMDb::Movie_AVG.end(); ++it) {
        if (it->second.movie_id == movie_id) {
            it->second.rating = film_rating;
        }
    }
}

void IMDb::remove_rating(std::string user_id, std::string movie_id) {
    UserMoviePair x;
    x.user_id = user_id;
    x.movie_id = movie_id;

    int rating = IMDb::RatingsUM[x];
    IMDb::RatingsUM.erase(x);

    double film_rating = IMDb::FilmRatings[movie_id].OverallRating();
    IMDb::FilmRatings[movie_id].RemoveRating(rating);
    double new_film_rating = IMDb::FilmRatings[movie_id].OverallRating();

    int timestamp = IMDb::Films[movie_id].timestamp;

    std::vector<std::string> categs = IMDb::Films[movie_id].categories;
    for (auto it = categs.begin(); it != categs.end(); ++it) {
        IMDb::Categories[(*it)].RemoveRating(timestamp, film_rating);
        if (new_film_rating != -1) {
            IMDb::Categories[(*it)].AddRating(timestamp, new_film_rating);
        }
    }


    for (auto it = mR.begin(); it != mR.end(); ++it) {
        if (it->movie_id == movie_id) {
            it->rating_number = IMDb::FilmRatings[movie_id].rating_number;
        }
    }

    for (auto it = IMDb::Movie_AVG.begin(); it != IMDb::Movie_AVG.end(); ++it) {
        if (it->second.movie_id == movie_id) {
            it->second.rating = new_film_rating;
        }
    }
}

std::string IMDb::get_rating(std::string movie_id) {
    double med = IMDb::FilmRatings[movie_id].OverallRating();
    if (med != -1) {
        char buff[10];
        snprintf(buff, sizeof(buff), "%.2lf", med);
        std::string a(buff);  // = std::to_string(med);
        return a;
    }
    return "none";
}

std::string IMDb::get_longest_career_actor() {
    int maxCareer = -1;
    std::string actor = "none";
    for (auto it = ActorMovies.begin(); it != ActorMovies.end(); ++it) {
        if (it->second.ActingCareer() > maxCareer) {
            maxCareer = it->second.ActingCareer();
            actor = it->first;
        }
    }
    return actor;
}

std::string IMDb::get_most_influential_director() {
    unsigned int maxNumber = 0;
    std::string director;
    if (Directors.empty()) {
        return "none";
    }
    for (auto it = Directors.begin(); it != Directors.end(); ++it) {
        if (it->second.actors_ids.size() > maxNumber) {
            maxNumber = it->second.actors_ids.size();
            director = it->first;
        }
    }

    return director;
}

std::string IMDb::get_best_year_for_category(std::string category) {
    int year = IMDb::Categories[category].GetBestYear();
    if (year != 1899) {
        return std::to_string(year);
    }
    return "none";
}

std::string IMDb::get_2nd_degree_colleagues(std::string actor_id) {
    std::set<std::string> solution_set;
    if (Colleagues[actor_id].empty()) {
        return "none";
    }
    for (auto it = Colleagues[actor_id].begin();
     it != Colleagues[actor_id].end(); ++it) {
        for (auto iter = Colleagues[*it].begin();
         iter != Colleagues[*it].end(); ++iter) {
            if (*iter != actor_id) {
                if (Colleagues[actor_id].find(*iter) ==
                 Colleagues[actor_id].end()) {
                    solution_set.insert(*iter);
                }
            }
        }
    }

    if (solution_set.empty()) {
        return "none";
    }

    std::string solution_string;
    std::vector<std::string> copy;
    for (auto it = solution_set.begin(); it != solution_set.end(); ++it) {
        copy.push_back(*it);
    }
    auto first_element = copy.begin();
    solution_string = solution_string + (*first_element);
    for (auto it = copy.begin() + 1; it != copy.end(); ++it) {
        solution_string = solution_string + " " + (*it);
    }

    return solution_string;
}

std::string IMDb::get_top_k_most_recent_movies(int k) {
    if (mT.empty()) {
    return "none";
    }

    if (k > (int)mT.size()) {
        k = mT.size();
    }
    std::partial_sort(mT.begin(), mT.begin() + k, mT.end());

    std::string solution;
    solution = (mT.begin())->movie_id;
    --k;
    for (auto it = mT.begin() + 1; it != mT.end() && k; ++it, --k) {
        solution = solution + " " + it->movie_id;
    }
    return solution;
}

std::string IMDb::get_top_k_actor_pairs(int k) {
    std::vector<cooperation> solution;

    if (Coops.empty()) {
    return "none";
    }
    for (auto it = Coops.begin(); it != Coops.end(); ++it) {
        solution.push_back(cooperation(*it));
    }

    if (solution.empty()) {
    return "none";
    }

    if (k > (int)solution.size()) {
        k = solution.size();
    }
    std::partial_sort(solution.begin(), solution.begin() + k, solution.end());

    auto first_element = solution.begin();
    if (solution.size() != 0) {
        std::string sol_string;
        sol_string = sol_string + "(" + first_element->actor1 + " ";
        sol_string = sol_string + first_element->actor2 + " ";
        sol_string = sol_string + std::to_string(first_element->movies) + ")";
        k--;
        for (auto it = solution.begin() + 1; it != solution.end() && k;
         ++it, --k) {
            sol_string = sol_string + " (" + it->actor1 + " ";
            sol_string = sol_string + it->actor2 + " ";
            sol_string = sol_string + std::to_string(it->movies) + ")";
        }
        return sol_string;
    }
    return "none";
}

std::string IMDb::get_top_k_partners_for_actor(int k, std::string actor_id) {
    std::vector<coactor> solution;

    if (Colleagues[actor_id].empty()) {
    return "none";
    }
    for (auto it = Colleagues[actor_id].begin();
     it != Colleagues[actor_id].end(); ++it) {
        std::string actor_id2 = *it;
        actorPair x(actor_id, actor_id2);
        int number_of_movies = Coops[x];
        solution.push_back(coactor(actor_id2, number_of_movies));
    }
    if (solution.empty()) {
    return "none";
    }

    if (k > (int)solution.size()) {
        k = solution.size();
    }
    std::partial_sort(solution.begin(), solution.begin() + k, solution.end());
    if (solution.empty()) {
	return "none";
    }
    std::string sol_string;
    sol_string = sol_string + (solution.begin())->actor;
    k--;
    for (auto it = solution.begin() + 1; it != solution.end() && k; ++it, --k) {
        sol_string = sol_string + " " + it->actor;
    }
    return sol_string;
    return "none";
}

std::string IMDb::get_top_k_most_popular_movies(int k) {
    if (mR.empty()) {
    return "none";
    }

    if (k > (int)mR.size()) {
        k = mR.size();
    }
    std::partial_sort(mR.begin(), mR.begin() + k, mR.end());

    std::string solution;
    solution = (mR.begin())->movie_id;
    --k;
    for (auto it = mR.begin() + 1; it != mR.end() && k; ++it, --k) {
        solution = solution + " " + it->movie_id;
    }
    return solution;
}

std::string IMDb::get_avg_rating_in_range(int start, int end) {
    double AVG = 0;
    int number = 0;

    if (IMDb::Movie_AVG.empty()) {
    return "none";
    }
    for (auto it = IMDb::Movie_AVG.begin(); it != IMDb::Movie_AVG.end(); ++it) {
        if (it->first >= start && it->first <=end) {
            std::string id = it->second.movie_id;
            double rating = FilmRatings[id].OverallRating();
            if (rating >= 0) {
                AVG += rating;
                ++number;
            }
        }
    }


    if (number == 0) {
        return "none";
    }

    AVG = (double)AVG / number;
    char buff[10];
    snprintf(buff, sizeof(buff), "%.2lf", AVG);

    std::string ANSWER(buff);

    return ANSWER;
}
