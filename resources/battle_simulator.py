from random import randint

def calculate_exp_gain(level, lucky):
    effort_value = randint(100, 300)
    return effort_value * level * (2 if lucky else 1) // 6

def simulate_battles(show_battles=False):
    level = 1
    exp = 0
    num_battles = 0
    prev_battles = -1
    while level < 100:
        exp_gain = calculate_exp_gain(randint(level - 4, level + 5), level > 30)
        next_level = (level + 1)**3
        while exp_gain > 0:
            exp_gain -= 1
            exp += 1
            if (exp == next_level):
                level += 1
                next_level = (level + 1)**3
                if show_battles:
                    print(f'Level up! From {level-1}->{level}, took {num_battles - prev_battles} battles')
                prev_battles = num_battles
            # if show_battles:
                # print(f'{exp_gain}exp remaining')
        num_battles += 1
        # if show_battles:
        #     print(f'Battle {num_battles}, level {level}')
        # print(level)
    return num_battles

if __name__ == "__main__":
    num_simulations = 100
    total = 0
    for i in range(100):
        battles = simulate_battles(True)
        print(f'{battles} battles')
        total += battles
    print(f'On average, takes {total // num_simulations} battles')